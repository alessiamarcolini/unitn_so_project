//
// Created by Alessia Marcolini on 2019-05-23.
//

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

bool switchLabel(char * id, char label, char position, long childrenPids[MAXLEN]) {
    char message[MAXLEN];

    sprintf(message, "%s down 0 %c;%c", id, label, position);

    bool status = writeAllChildren(message, childrenPids);
    return status;
}

pid_t spawn(int type, char * id, long childrenPids[MAXLEN], int * firstFreePosition) {

    pid_t pid = fork();

    if (pid == 0) { // child

        char * pipeName = getPipename((long) getpid());
        mkfifo(pipeName, 0777);


        // switch type
        char * bin;

        switch (type){
            case BULB:
                bin = BIN_BULB;
                break;

            case HUB:
                bin = BIN_HUB;
                break;
        }

        char * const paramList[] = {bin, id, NULL}; // type 2
        int e = execv(paramList[0], paramList);
        if (e < 0){
            printf( "Error execv: %s\n", strerror( errno ) );
        }


    }
    else {
        childrenPids[*firstFreePosition] = (long) pid;
        * firstFreePosition = calculateNewFreePosition(childrenPids, *firstFreePosition);

        if (* firstFreePosition == -1){
            printf("Ok but no room for other children");

        }


    }
    return pid;
}

bool info(char * id, long childrenPids[MAXLEN], bool * waitingResponse) {

    // TODO: check ID
    *waitingResponse = true;
    char message[MAXLEN];

    sprintf(message, "%s down 0 %d", id, INFO);

    bool status = writeAllChildren(message, childrenPids); // manage status below ?

    return status;
}

bool writeAllChildren(char message[MAXLEN], long childrenPids[MAXLEN]){

    // TODO: manage status
    bool status = true;

    int fd;
    char *pipeName;

    int i;
    for (i = 0; i < MAXLEN; i++) {
        if (childrenPids[i] != -1) {
            printf("send to pid: %ld\n", childrenPids[i]);


            //kill((pid_t) childrenPids[i], SIGUSR2);
            char *pipeName = getPipename(childrenPids[i]);

            kill((pid_t) childrenPids[i], SIGUSR1);

            int fd = open(pipeName, O_RDWR);

            while (fd < 0) {
                sleep(1);
                fd = open(pipeName, O_RDWR);
                printf("Error opening file: %s\n", strerror(errno));
                return false;
            }

            write(fd, message, strlen(message) + 1);
            sleep(1);

            close(fd);


        }


    }
    sleep(2);
    return status;
}
