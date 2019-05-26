//Library containing methods shared between controller devices e.g. Hubs and Main Controller

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

//Inner level command that prepares label switching message and uses "writeAllChildren" function to forward it to all direct children of the current device
//returns true if the messages were successfully sent
bool switchLabel(char * id, char label, char position, long childrenPids[MAXLEN]) {
    char message[MAXLEN];

    sprintf(message, "%s down 0 %c;%c", id, label, position);

    bool status = writeAllChildren(message, childrenPids);
    return status;
}

//spawns a child process of the current controller and makes it run an execv to turn it into the desired device
//returns the process id of the spawned device
pid_t spawn(int type, char * id, long childrenPids[MAXLEN], int * firstFreePosition) {

    pid_t pid = fork();

    if (pid == 0) { //child's code

        char * pipeName = getPipename((long) getpid());
        mkfifo(pipeName, 0777);


        //switch on device type
        char * bin;

        switch (type){
            case BULB:
                bin = BIN_BULB;
                break;

            case HUB:
                bin = BIN_HUB;
                break;
        }
        //telling the child process its type and its id
        char * const paramList[] = {bin, id, NULL};
        int e = execv(paramList[0], paramList);
        if (e < 0){
            printf( "Error execv: %s\n", strerror( errno ) );
        }
    }
        //saves newborn's pid in the first free position of the array containing direct children
        else {
        childrenPids[*firstFreePosition] = (long) pid;
        * firstFreePosition = calculateNewFreePosition(childrenPids, *firstFreePosition);

        if (* firstFreePosition == -1){ //array is full so we print an error message
            printf("Ok but no room for more children");
        }
    }
    return pid;
}

//Prepares the message to be forwarded to all children, sets the waitingResponse flag to true
//returns true if messages were sent correctly, false otherwise
bool info(char * id, long childrenPids[MAXLEN], bool * waitingResponse) {

    // TODO: check ID
    *waitingResponse = true;
    char message[MAXLEN];

    sprintf(message, "%s down 0 %d", id, INFO);

    bool status = writeAllChildren(message, childrenPids);

    return status;
}

//sends a message, taken as a parameter, to all pids contained in the given array
//returns true if messages were sent correctly, false otherwise
bool writeAllChildren(char message[MAXLEN], long childrenPids[MAXLEN]){

    // TODO: manage status
    bool status = true;

    int fd;
    char *pipeName;

    int c; // number of written characters

    //cycles on all active children pids sending the message
    int i;
    for (i = 0; i < MAXLEN; i++) {

        if (childrenPids[i] != -1) { //pid = -1 means no child in that position

            //prepare pipename
            char *pipeName = getPipename(childrenPids[i]);

            //tell the child process it needs to check his incoming pipe for new messages
            kill((pid_t) childrenPids[i], SIGUSR1);

            //open the pipe
            int fd = open(pipeName, O_RDWR);

            //retry to open the pipe
            while (fd < 0) {
                sleep(1);
                fd = open(pipeName, O_RDWR);
                printf("Error opening file: %s\n", strerror(errno));
                return false;
            }
            //write the message into the pipe
            c = write(fd, message, strlen(message) + 1);
            if (c == -1){
                status = false;
            }
            sleep(1);
            close(fd);
        }
    }
    sleep(2);
    return status;
}
