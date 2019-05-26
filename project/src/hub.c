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

//global variables
bool result = true;

pid_t pid;
pid_t ppid;

int id;
int deviceType = HUB;

//a controller device holds a number of children
long childrenPids[MAXLEN];
int firstFreePosition = 0;

int fdUp;
int fdIn;

char * fifoIn;
char * fifoUp;

int idReceiver;
int idSender;

int sigIn = -1;


int numChildren = 0;

bool waitingResponse = false; // TODO: counter?

//sends a message up the tree (to its parent)
//returns true if the send was successful
bool forwardUp(char message[MAXLEN]){
    //printf("forward up\n");

    bool status = true;

    fdUp = open(fifoUp, O_RDWR);
    kill(ppid, SIGUSR1);
    write(fdUp, message, strlen(message) + 1);
    sleep(1);
    close(fdUp);

    return status;
}

//sends a message down the tree (to all the active children if any)
//returns true if all sends were successful
bool forwardDown(char message[MAXLEN]){
    //printf("forward down\n");

    bool status = true;
    status = writeAllChildren(message, childrenPids);

    return status;
}

//signal handler, behaves differently based on the type of message received
void handleSignal(int sig) {

    sigIn = sig;

    //if the signal was an "ack" then we can simply forward it up without further investigation
    if (sig == SIGUSR2){
        kill(ppid, SIGUSR2);
        return;
    }

    char tmp[MAXLEN];

    //open pipe
    fdIn = open(fifoIn, O_RDWR);

    while (fdIn < 0) {
        printf("Error opening pipe to bulb with id: %d and pid: %ld", id, (long) pid);
        fdIn = open(fifoIn, O_RDWR);
    }

    //read message from pipe
    while (read(fdIn, tmp, MAXLEN | O_NONBLOCK) == -1) {
    };

    char buf[MAXLEN];

    close(fdIn);

    char * message[MAXLEN];

    char tmpCopy[MAXLEN];
    strcpy(tmpCopy, tmp);

    //split incoming string into major tokens to detect main pieces of information
    tokenizer(tmp, message, " ");


    idReceiver = atoi(message[0]);
    idSender = atoi(message[2]);

    //if the message is for this device then we proceed to execute it
    if (idReceiver == id || idReceiver == BROADCAST_ID) {
        char *commands[MAXLEN];

        //split the last major token into minor tokens to get details about the command
        tokenizer(message[3], commands, ";");

        //if it's a status command then we need to send a similar command to all of our children
        if (commands[0][0] == STATUS_S) {
            int tmpStatus;

            if (commands[1][0] == OFF_S) {
                tmpStatus = OFF;
            }
            if (commands[1][0] == ON_S) {
                tmpStatus = ON;

            }

            result = switchLabel(BROADCAST_ID_S, STATUS_S, tmpStatus, childrenPids);

            if (!result) {
                printf("Switch error\n");
            }
        //if it's an info command then we need to ask all of our children for their status in order to elaborate and send a response
        } else if (commands[0][0] == INFO_S) {
            printf("Operation not implemented yet\n");
            /*
            result = info(BROADCAST_ID_S, childrenPids, &waitingResponse); // TODO: counter variable instead of bool
            if (!result){
                printf("info error\n");
            }

            // TODO: manage responses

             */
        //if it's a spawn command then we create the desired device and link it to the current hub
        } else if (commands[0][0] == SPAWN_S) {

            char *idChild = commands[1];
            int childType = atoi(commands[2]);

            pid_t pidChild = spawn(childType, idChild, childrenPids, &firstFreePosition);
            if (pidChild == -1) {
                result = false;
                printf("Error: unable to create a new process\n");
            } else {
                numChildren++;

            }
        }

    //the command is not for this device, so we forward it up or down
    } else {

        //int c = sprintf(buf, "---- %d: not for me: %s!\n", id, tmp);
        //write(1, buf, c);

        //if the message is travelling up the tree then we forward it to our parent
        if (strcmp(message[1], "up") == 0) {
            result = forwardUp(tmpCopy);
            if (!result){
                printf("Internal forwarding up message error\n");
            }

        //if the message is travelling down the tree then we forward it to all our children
        } else if (strcmp(message[1], "down") == 0) {
            result = forwardDown(tmpCopy);
            if (!result){
                printf("Internal forwarding down message error\n");
            }

        } else {
            char alert[MAXLEN];
            int c = sprintf(alert, "Error formatting the message: direction %s not allowed!\n", message[1]);
            write(1, alert, c);

        }
    }
}


//prepare the device and set it up to receive messages
int main(int argc, char * argv[]){

    initChildren(childrenPids);

    pid = getpid();
    ppid = getppid();

    id = atoi(argv[1]);

    fifoIn = getPipename((long) pid); //in-pipe
    fifoUp = getPipename((long) ppid); //out-pipe with my parent

    //set up signal handlers
    struct sigaction psa1;
    psa1.sa_handler = handleSignal;
    psa1.sa_flags = SA_ONSTACK;
    sigaction(SIGUSR1, &psa1, NULL);
    sigaction(SIGUSR2, &psa1, NULL);

    static char stack[SIGSTKSZ];
    stack_t ss = {
            .ss_size = SIGSTKSZ,
            .ss_sp = stack,
    };
    sigaltstack(&ss, 0);

    sigset_t myset;
    (void) sigemptyset(&myset);

    //send parent an "ack" signal to tell it that this device is up and running, ready to handle messages
    kill(ppid, SIGUSR2);

    //wait for signals
    while (1) {

        (void) sigsuspend(&myset);
        if (!result){
            printf("Error\n");
        }
    }
    return 0;
}