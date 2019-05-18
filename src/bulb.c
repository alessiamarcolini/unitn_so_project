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

/*
<command>
0: off
1: on
*/


int status = 0; // off at start
time_t startTime;

int fd; // pipe file descriptor


void handleSignal(int sig) {
    char tmp[MAXLEN];

    read(fd, tmp, MAXLEN);
    char **commands;
    tokenizer(tmp, commands);


    if (atoi(commands[0]) == 0) { // off
        if (!status) {
            status = 1;
            startTime = time(NULL); // get current timestamp
        } else { // on
            status = 0;
            startTime = 0;
        }
    }
}

int main(int argc, char * argv[]){



    pid_t pid = getpid();

    int index = atoi(argv[1]);

    // FIFO file path
    char * fifo = getPipename(pid);

    fd = open(fifo, O_RDWR);

    signal(SIGUSR1, handleSignal);

    while(1)
        sleep(1);

    return 0;
}