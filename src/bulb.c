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

int sigIn = -1;

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


void handleSignal(int sig) {

    printf("ok sig bulb received %d\n", sig);

    sigIn = sig;

    if (sig == SIGUSR2){
        return;
    }
    /*char alert[MAXLEN];
    int c = sprintf(alert, "SONO BULB %d PID %d SIG %d\n", id, (int) pid, (int) sig);
    write(1, alert, c);*/

    int c;
    char buf[MAXLEN];

    sigIn = sig;


    char tmp[MAXLEN];

    fdIn = open(fifoIn, O_RDWR); // open pipe -

    c = sprintf(buf, "---- %d: fdin open: %d!\n", id, fdIn);
    write(1, buf, c);


    while (fdIn < 0){
        printf("Error opening pipe to bulb with id: %d and pid: %ld", id, (long) pid);
        fdIn = open(fifoIn, O_RDWR);
    }


    while (read(fdIn, tmp, MAXLEN | O_NONBLOCK) == -1){
        printf("---- %d: non c'è niente da leggere\n", id);
    };


    c = sprintf(buf, "---- %d: letto: %s!\n", id, tmp);
    write(1, buf, c);
    //printf("---- %d: letto: %s!\n", id, tmp);


    //char buf[MAXLEN+30];
    //int c = sprintf(buf, "---- %d: letto: %s!\n", id, tmp);
    //write(1, buf, c);
    //printf("---- %d: letto: %s!\n", id, tmp);


    close(fdIn);
    char * message[MAXLEN];
    tokenizer(tmp, message, " ");


    idReceiver = atoi(message[0]);
    idSender = atoi(message[2]);


    if (idReceiver == id || idReceiver == BROADCAST_ID) { // message for me, or "broadcast"

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
    return;
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




    struct sigaction psa1;
    psa1.sa_handler = handleSignal;
    psa1.sa_flags = SA_ONSTACK;
    sigaction(SIGUSR1, &psa1, NULL);
    sigaction(SIGUSR2, &psa1, NULL);

    //setting "handler" for broken pipes
    struct sigaction psa2;
    psa2.sa_handler = SIG_IGN;
    sigaction(SIGPIPE, &psa2, NULL);

    static char stack[SIGSTKSZ];
    stack_t ss = {
            .ss_size = SIGSTKSZ,
            .ss_sp = stack,
    };
    sigaltstack(&ss, 0);


    sigset_t myset;
    (void) sigemptyset(&myset);

    //setting "handler" for broken pipes


    kill(ppid, SIGUSR2); // I'm alive

    while (1) {

        (void) sigsuspend(&myset);
        printf("Bulb id %d --- Letto signal: %d\n", id, sigIn);

    }

    return 0;
}
