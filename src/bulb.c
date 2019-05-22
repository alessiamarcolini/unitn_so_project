//
// Created by Alessia Marcolini on 2019-05-01.
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <time.h>

#include "utils.h"



// protocol first try

// [Receiver] [up/down] [Sender] <command;specs>



int status;
time_t startTime = (time_t) -1;

int id;
int deviceType = BULB;

pid_t pid;
pid_t ppid;

int fdUp; // pipe file descriptor
int fdIn;

char * fifoIn;
char * fifoUp;

int idReceiver;
int idSender;

void handleSignalKnock(int sig);

void handleSignal(int sig) {

    signal(SIGUSR1, handleSignal);


    char tmp[MAXLEN];

    fdIn = open(fifoIn, O_RDONLY); // open pipe -

    while (fdIn < 0){
        fdIn = open(fifoIn, O_RDONLY);
        printf("Error opening pipe to bulb with id: %d and pid: %ld", id, (long) pid);
    }

    while (read(fdIn, tmp, MAXLEN | O_NONBLOCK) == -1);
    close(fdIn);


    char * message[MAXLEN];
    tokenizer(tmp, message, " ");


    idReceiver = atoi(message[0]);
    idSender = atoi(message[2]);


    if (idReceiver == id) { // messaggio per me


        char * commands[MAXLEN];
        tokenizer(message[3], commands, ";");


        if (commands[0][0] == STATUS_S) {
            if (commands[1][0] == OFF_S) {
                status = OFF;
                startTime = (time_t) -1;
            }
            if (commands[1][0] == ON_S) {
                status = ON;
                startTime = time(NULL); // get current timestamp

            }

        }



        else if (commands[0][0] == INFO_S){


            // preparing message
            char msg[MAXLEN];

            int activeTime;
            if (startTime != (time_t) -1){
                activeTime = (int) difftime(time(NULL), startTime);
            }
            else{
                activeTime = 0;
            }


            sprintf(msg, "%d up %d %d;%d;%d;%d", idSender, id, INFO_BACK, deviceType, status, activeTime);





            fdUp = open(fifoUp, O_RDWR);


            kill(ppid, SIGUSR1);


            write(fdUp, msg, strlen(msg) + 1);
            sleep(1);
            close(fdUp);
            

        }


        else {
            printf("Command not allowed.\n");  // send back switch error
        }



    }




}

    int main(int argc, char * argv[]){


    status = OFF;

    pid = getpid();
    ppid = getppid();

    id = atoi(argv[1]);

    // FIFO file path
    fifoIn = getPipename((long) pid); // in-pipe read only
    fifoUp = getPipename((long) ppid); // out-pipe with my parent write only

    //signal(SIGUSR1, handleSignal);
    //signal(SIGUSR2, handleSignalKnock);




    struct sigaction psa;
    psa.sa_handler = handleSignal;
    sigaction(SIGUSR1, &psa, NULL);
    sigaction(SIGUSR2, &psa, NULL);





    sigset_t myset;
    (void) sigemptyset(&myset);
    while (1) {
        (void) sigsuspend(&myset);
    }

    return 0;
}
