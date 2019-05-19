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



/*
<command>
0: off
1: on
*/


int status;
time_t startTime;

int fdUp; // pipe file descriptor
int fdIn;

char * fifoIn;
char * fifoUp;

int id;

int idReceiver;
int idSender;


void handleSignal(int sig) {
    char tmp[MAXLEN];

    fdIn = open(fifoIn, O_RDONLY | O_NONBLOCK); // open pipe - non-blocking

    while (read(fdIn, tmp, MAXLEN) != -1) {

        char **message;
        tokenizer(tmp, message, " ");

        idReceiver = atoi(message[0]);

        if (idReceiver == id) { // messaggio per me

            char ** commands;
            tokenizer(message[3], commands, ";");


            if (commands[0][0] == STATUS_S){
                if (commands[1][0] == OFF_S){
                    status = OFF;
                    startTime = 0;
                }
                if (commands[1][0] == ON_S){
                    status = ON;
                    startTime = time(NULL); // get current timestamp
                }

                else {
                    printf("Command not allowed.\n");
                }
            }


        }
    }

    close(fdIn);
}

    int main(int argc, char * argv[]){

    status = OFF;

    pid_t pid = getpid();
    pid_t ppid = getppid();

    id = atoi(argv[1]);

    // FIFO file path
    fifoIn = getPipename(pid); // in-pipe read only
    fifoUp = getPipename(ppid); // out-pipe with my parent write only

    fdUp = open(fifoUp, O_WRONLY); // NO è da aprire se devo comunicare

    signal(SIGUSR1, handleSignal);

    while(1)
        sleep(1);

    return 0;
}