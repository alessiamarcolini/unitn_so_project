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

//status global variables such as registers and ids
int status;
time_t startTime = (time_t) -1;

int sigIn = -1;

int id;
int deviceType = BULB;

pid_t pid;
pid_t ppid;

//file descriptors
int fdUp;
int fdIn;

char * fifoIn;
char * fifoUp;

int idReceiver;
int idSender;

//signal handler, behaves in different ways based on the incoming fifo content
void handleSignal(int sig) {
    //store signal number to global variable
    sigIn = sig;

    if (sig == SIGUSR2){
        return;
    }

    int c;
    char buf[MAXLEN];

    sigIn = sig;


    char tmp[MAXLEN];

    fdIn = open(fifoIn, O_RDWR); //open pipe

    //c = sprintf(buf, "---- %d: fdin open: %d!\n", id, fdIn);
    //write(1, buf, c);

    //retry to open pipe
    while (fdIn < 0){
        printf("Error opening pipe to bulb with id: %d and pid: %ld", id, (long) pid);
        fdIn = open(fifoIn, O_RDWR);
        sleep(1);
    }

    //try to read the incoming message and store it in tmp
    while (read(fdIn, tmp, MAXLEN | O_NONBLOCK) == -1);

    close(fdIn);
    char * message[MAXLEN];

    //splits the incoming message into 4 major tokens
    tokenizer(tmp, message, " ");

    idReceiver = atoi(message[0]);
    idSender = atoi(message[2]);

    //if the message is intended to be opened in this device then it is opened inside this statement
    //there is no "else" statement since this is an end device, so no message should be forwarded in any direction
    if (idReceiver == id || idReceiver == BROADCAST_ID) { // message for me, or "broadcast"

        char * commands[MAXLEN];
        //splits the last major token, which contained the details of the command
        tokenizer(message[3], commands, ";");

        //handle switch status command
        if (commands[0][0] == STATUS_S) {

            if (commands[1][0] == OFF_S) {
                status = OFF;
                startTime = (time_t) -1;
            }
            if (commands[1][0] == ON_S) {
                status = ON;
                startTime = time(NULL); //get current timestamp
            }

        }


        //respond to a request for info command
        else if (commands[0][0] == INFO_S){

            char msg[MAXLEN];

            //active time tells how many seconds elapsed since the last time the bulb was turned on
            int activeTime;
            if (startTime != (time_t) -1){
                activeTime = (int) difftime(time(NULL), startTime);
            }
            else{
                activeTime = 0;
            }

            sprintf(msg, "%d up %d %d;%d;%d;%d", idSender, id, INFO_BACK, deviceType, status, activeTime);

            //open fifo, tell parent to listen (kill), write into fifo and close it.
            fdUp = open(fifoUp, O_RDWR);

            kill(ppid, SIGUSR1);

            write(fdUp, msg, strlen(msg) + 1);
            sleep(1);
            close(fdUp);
        }
        //command is under construction or not allowed
        else {
            printf("Command not allowed.\n");
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
    fifoIn = getPipename((long) pid); //in-pipe
    fifoUp = getPipename((long) ppid); //out-pipe with my parent

    //set up signal handler
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

    //this kill tells parent that this device is ready to handle signals
    kill(ppid, SIGUSR2);

    while (1) {
        (void) sigsuspend(&myset);

    }

    return 0;
}
