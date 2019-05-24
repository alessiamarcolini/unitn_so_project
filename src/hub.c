#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "utils.h"
#include "controllerActions.h"


bool result = true;

pid_t pid;
pid_t ppid;

int id;
int deviceType = HUB;

long childrenPids[MAXLEN]; // you can now calculate pipe names
int firstFreePosition = 0; // of the children's pid array - useful for inserting the next one without scanning all the array

int fdUp;
int fdIn;

char * fifoIn;
char * fifoUp;

int idReceiver;
int idSender;

int sigIn = -1;


int numChildren = 0;

bool waitingResponse = false; // TODO: counter?

bool forwardUp(char message[MAXLEN]){
    printf("forward up\n");

    bool status = true;

    fdUp = open(fifoUp, O_RDWR);
    kill(ppid, SIGUSR1);
    write(fdUp, message, strlen(message) + 1);
    sleep(1);
    close(fdUp);


    return status;
}

bool forwardDown(char message[MAXLEN]){
    printf("forward down\n");

    bool status = true;
    status = writeAllChildren(message, childrenPids);

    return status;
}




void handleSignal(int sig) {

    printf("ok sig hub received %d\n", sig);
    sigIn = sig;
    if (sig == SIGUSR2){
        kill(ppid, SIGUSR2);

        return;
    }

    sigIn = sig;

    char tmp[MAXLEN];

    fdIn = open(fifoIn, O_RDONLY); // open pipe

    while (fdIn < 0) {
        printf("Error opening pipe to bulb with id: %d and pid: %ld", id, (long) pid);
        fdIn = open(fifoIn, O_RDONLY);
    }


    while (read(fdIn, tmp, MAXLEN | O_NONBLOCK) == -1) {
    };

    char buf[MAXLEN];

    close(fdIn);

    char * message[MAXLEN];

    tokenizer(tmp, message, " ");


    idReceiver = atoi(message[0]);
    idSender = atoi(message[2]);

    if (idReceiver == id || idReceiver == BROADCAST_ID) {
        char *commands[MAXLEN];
        tokenizer(message[3], commands, ";");


        if (commands[0][0] == STATUS_S) {
            int tmpStatus;

            if (commands[1][0] == OFF_S) {
                tmpStatus = OFF;
            }
            if (commands[1][0] == ON_S) {
                tmpStatus = ON;

            } // manda a tutti

            result = switchLabel(BROADCAST_ID_S, STATUS_S, tmpStatus, childrenPids);

            if (!result) {
                printf("Switch error\n");
            }

        } else if (commands[0][0] == INFO_S) { // TODO: chiedere a tutti
            result = info(BROADCAST_ID_S, childrenPids, &waitingResponse); // TODO: counter invece che bool
            if (!result){
                printf("info error\n");
            }

            // TODO: gestire risposte

        } else if (commands[0][0] == SPAWN_S) { // TODO: non riceve bene

            char *idChild = commands[1];
            int childType = atoi(commands[2]);

            pid_t pidChild = spawn(childType, idChild, childrenPids, &firstFreePosition);
            if (pidChild == -1) {
                result = false;
                printf("spawn error\n");
            } else {
                numChildren++;

            }
        }


    } else { // not for me - then forward
        // controlla up o down

        int c = sprintf(buf, "---- %d: not for me: %s!\n", id, tmp);
        write(1, buf, c);

        if (strcmp(message[1], "up") == 0) {
            //  || strcmp(message[1], "down") == 0) {
            result = forwardUp(tmp);
            if (!result){
                printf("forward up error\n");
            }


        } else if (strcmp(message[1], "down") == 0) {
            result = forwardDown(tmp);
            if (!result){
                printf("forward down error\n");
            }

        } else {
            char alert[MAXLEN];
            int c = sprintf(alert, "Error formatting the message: direction %s not allowed!\n", message[1]);
            write(1, alert, c);

        }

    }

}



int main(int argc, char * argv[]){

    initChildren(childrenPids);

    pid = getpid();
    ppid = getppid();

    id = atoi(argv[1]);

    fifoIn = getPipename((long) pid); // in-pipe read only
    fifoUp = getPipename((long) ppid); // out-pipe with my parent write only



    struct sigaction psa1;
    psa1.sa_handler = handleSignal;
    sigaction(SIGUSR1, &psa1, NULL);
    sigaction(SIGUSR2, &psa1, NULL);


    sigset_t myset;
    (void) sigemptyset(&myset);


    kill(ppid, SIGUSR2); // I'm alive


    while (1) {

        (void) sigsuspend(&myset);
        if (!result){
            printf("Error\n");
        }
        else {
            printf("Hub id %d --- Letto signal: %d\n", id, sigIn);
        }

    }




    return 0;
}