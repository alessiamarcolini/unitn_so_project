#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <signal.h>

#include "utils.h"
#include "limb.h"
#include "controllerActions.h"

//this file contains the main controller and the user interaction part

limb * limbo;

//the id of the main controller is always zero
int id = 0;
pid_t pid;

int fdIn;
char * fifoIn;

//this flag will be set to true if the running command needs to wait for messages from the subtree before resuming its execution
//when set to true disables the user input
bool waitingResponse = false;

int idReceiver;
int idSender;

// structure containing children's pid
long childrenPids[MAXLEN];
int firstFreePosition = 0; // of the children's pid array - useful for inserting the next one without scanning all the array
int numChildren = 0;

//print the tree of all processes and pending devices
void list(){

    printf("Added Devices list:\n");
    printLimb(limbo);

    // TODO: ask each children info
    printf("Linked Devices not available\n");
}

//add a new device to the system
//note that this function does not actually spawn a new process
//returns true if the operation was successful, false otherwise
bool add(char device[MAXLEN], int * id){
    bool status = true;

    (*id)++;
    //switch on device type, prepare the limb device
    if(strcmp(device, "hub\n") == 0) {

        limbDevice * tmp = (limbDevice *) malloc(sizeof(limbDevice));

        if (tmp != NULL){

            tmp->id = *id;
            tmp->fId = -1;
            tmp->type = HUB;
            tmp->next = NULL;
            tmp->registers = NULL;

            if (isLimbEmpty(limbo)){
                limbo->head = tmp;
                limbo->tail = tmp;
            }
            else {
                limbo->tail->next = tmp;
                limbo->tail = tmp;
            }

        }
        else {

            printf("Fatal: failed to add hub\n");
            status = false;
        }

    }

    else if(strcmp(device, "timer\n") == 0) {
        printf("Device not implemented yet\n");
        status = false;
    }

    else if(strcmp(device, "bulb\n") == 0) {

        limbDevice * tmp = (limbDevice *) malloc(sizeof(limbDevice));

        if (tmp != NULL){
            tmp->id = *id;
            tmp->fId = -1;
            tmp->type = BULB;
            tmp->next = NULL;
            tmp->registers = NULL;

            if (isLimbEmpty(limbo)){
                limbo->head = tmp;
                limbo->tail = tmp;
            }
            else {
                limbo->tail->next = tmp;
                limbo->tail = tmp;
            }

        }
        else {

            printf("Fatal: failed to add bulb\n");
            status = false;
        }
    }

    else if(strcmp(device, "window\n") == 0) {
        printf("Device not implemented yet\n");
        status = false;
    }
    else if(strcmp(device, "fridge\n") == 0) {
        printf("Device not implemented yet\n");
        status = false;
    }

    //device not recognized
    else {
        status = false;
        (*id)--;
    }

    return status;
}

//main controller function that decides whether to spawn the device or delegate the task to a lower controller
bool tie(char * idChild, char * idParent, bool * waitingResponse){
    *waitingResponse = true;

    bool status = true;

    //check if device is ready to be linked
    limbDevice * tmp = exists(atoi(idChild), limbo);
    int childType;

    if (tmp == NULL){
        status = false;

    }

    //main controller will be the parent
    else if (atoi(idParent) == 0){

        childType = tmp->type;
        pid_t pidChild = spawn(childType, idChild, childrenPids, &firstFreePosition);
        if (pidChild == -1){
            status = false;
            printf("Error: unable to create a new process\n");
        }
        else {
            numChildren++;
            status = true;
        }

    }

    //main controller won't be the parent.
    //delegate the spawn to some other controller
    else {

        childType = tmp->type;
        char message[MAXLEN];
        sprintf(message, "%d down 0 %d;%s;%d", atoi(idParent), SPAWN, idChild, childType);

        status = writeAllChildren(message, childrenPids);
    }
    //remove device from limb since it should now be a running process linked to the tree
    if (tmp != NULL && !removeFromLimb(atoi(idChild), limbo)) {
        status = false;
        printf("Error removing from limb\n");
    }

    return status;
}

//signal handler
void handleSignal(int sig){

    //"ack" received, we can now move on to the next user entered operation
    if (sig == SIGUSR2){
        waitingResponse = false;
        return;
    }

    //prepare string to handle system-generated input
    char tmp[MAXLEN];
    int i;
    for (i=0; i<MAXLEN; i++){
        tmp[i] = '\0';
    }
    fdIn = open(fifoIn, O_RDWR); //open pipe

    //retry to open fifo until there's something in it
    while (fdIn < 0) {
        printf("Error opening pipe to bulb with id: %d and pid: %ld", id, (long) pid);
        fdIn = open(fifoIn, O_RDWR);
    }

    read(fdIn, tmp, MAXLEN);

    char * message[MAXLEN];
    //splits the incoming message into 4 major tokens
    tokenizer(tmp, message, " ");

    idReceiver = atoi(message[0]);
    idSender = atoi(message[2]);


    if (idReceiver == id) {  //the message is for the main controller, so process it
        char * commands[MAXLEN];

        //splits the last major token, which contains the details of the command
        tokenizer(message[3], commands, ";");

        //incoming response from the device we consulted
        //display the information
        if (commands[0][0] == INFO_BACK_S) { // TODO: check exceptions
            printf("   Device info: \n");
            printf("   - Id: %d\n", idSender);

            switch (atoi(commands[1])){

                case BULB:
                    printf("   - Type: Bulb\n");
                    printf("   - Status: ");

                    switch (atoi(commands[2])){ // <status>
                        case ON:
                            printf("ON\n");
                            break;

                        case OFF:
                            printf("OFF\n");
                            break;
                    }

                    time_t activeTime = (time_t) atoi(commands[3]);  // <activeTime>

                    int seconds = activeTime % 60;
                    int minutes = (activeTime/60) % 60;
                    int hours = (activeTime/3600) % 24;

                    printf ("   - Active Time: %dh %dm %ds\n", hours, minutes, seconds);

                    break;
            }

        }
    }
    waitingResponse = false;
    close(fdIn);
}

//initializes all the global variables and signal handlers
int main(int argc, char *argv[]) {

    initChildren(childrenPids);

    int id = 0;

    limbo = (limb *) malloc(sizeof(limb));
    limbo->head = NULL;
    limbo->tail = NULL;

    pid = getpid();

    fifoIn = getPipename(pid);
    mkfifo(fifoIn, 0777);

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

    //input buffer
    char buffer[MAXLEN];

    //contains status of miscellaneous operations
    int status = 1;

    //input cycle, priority is given to system generated requests
    while (1) {

        //if we are not waiting for some kind of response from inside the system then we can wait for the user to input a command
        while (!waitingResponse) {

            printf(" > ");

            //the user hits enter, we process the command
            if (fgets(buffer, MAXLEN, stdin) != NULL) {
                char *tokens[MAXLEN];

                //tokenize the input into words
                tokenizer(buffer, tokens, " ");

                removeNewLine(tokens[0]);

                //switch on command and call the proper method passing the remaining portion of the input if required
                if (strcmp(tokens[0], "list") == 0) {
                    list();

                } else if (strcmp(tokens[0], "add") == 0) {
                    if (tokens[1] != NULL) {

                        status = add(tokens[1], &id);
                        if (!status) {
                            printf("Device not recognized\n");
                        } else {
                            printf("Added new device with id %d - type %s", id, tokens[1]);
                        }

                    }

                } else if (strcmp(tokens[0], "link") == 0) {

                    if (tokens[1] != NULL &&
                        ((strcmp(tokens[2], "to") == 0) && tokens[3] != NULL)) { // TODO: check better if id is valid

                        status = tie(tokens[1], tokens[3], &waitingResponse);

                        if (!status) {
                            printf("Device with ID %s not found or already linked\n", tokens[1]);
                            waitingResponse = false;
                        } else {
                            printf("Linked\n");
                        }

                    }

                } else if (strcmp(tokens[0], "switch") == 0) {
                    if (tokens[1] != NULL && atoi(tokens[1]) > 0 && tokens[2] != NULL &&
                        tokens[3] != NULL) {

                        char *id = tokens[1];

                        char *labelStr = tokens[2];
                        char label;

                        char *positionStr = tokens[3];
                        removeNewLine(positionStr);
                        char position;

                        //label check
                        if (strcmp(labelStr, "status") == 0) {
                            label = STATUS_S;

                        } else {
                            printf("Label %s does not exist\n", tokens[2]);
                            break;
                        }

                        // position check
                        if (strcmp(positionStr, "on") == 0) {
                            position = ON_S;

                        } else if (strcmp(positionStr, "off") == 0) {
                            position = OFF_S;

                        } else {
                            printf("Position %s does not exist\n", tokens[3]);
                            break;
                        }

                        //ready to run command
                        status = switchLabel(id, label, position, childrenPids);

                        if (!status) {
                            printf("Switch error\n");
                        }
                        else{
                            printf("Switched\n");
                        }

                    }

                } else if (strcmp(tokens[0], "info") == 0) {
                    if (tokens[1] != NULL) {

                        removeNewLine(tokens[1]);
                        status = info(tokens[1], childrenPids, &waitingResponse);

                        if (!status) {
                            printf("Info error\n");
                        }
                    }

                }
                else if (strcmp(tokens[0], "del") == 0) {
                    printf("Operation not implemented yet\n");
                }


            } else {
                printf("Error reading from stdin!\n");
            }

        }

    }
    return 0;
}
