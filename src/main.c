//
// Created by Alessia Marcolini on 2019-04-29.
//

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

#define debug

limb * limbo;

int id = 0;
pid_t pid;

int fdIn;
char * fifoIn;

bool waitingResponse = false;

int idReceiver;
int idSender;

// structure containing children's pid
long childrenPids[MAXLEN]; // you can now calculate pipe names
int firstFreePosition = 0; // of the children's pid array - useful for inserting the next one without scanning all the array







pid_t spawn(int type, int id) {

    pid_t pid = fork();

    if (pid == 0) { // child

        char * pipeName = getPipename((long) getpid());
        mkfifo(pipeName, 0777);

        char deviceStr[MAXLEN];
        sprintf(deviceStr, "%d", id);

        // switch type
        char * const paramList[] = {"./bin/bulb", deviceStr, NULL}; // type 2
        int e = execv(paramList[0], paramList);
        if (e < 0){
            printf( "Error execv: %s\n", strerror( errno ) );
        }


    }
    else {
        childrenPids[firstFreePosition] = (long) pid;
        firstFreePosition = calculateNewFreePosition(childrenPids, firstFreePosition);

        if (firstFreePosition == -1){
            printf("Ok but no room for other children");

        }


    }
    return pid;
}

bool info(char * id){

    // TODO: check ID

    char message[MAXLEN];

    sprintf(message, "%s down 0 %d", id, INFO);

    bool status = true; // manage status below ?

    int fd;
    char * pipeName;

    int i;
    for (i = 0; i < MAXLEN; i++) {


        if (childrenPids[i] != -1) {
//            printf("%d\n", i);

            kill((pid_t) childrenPids[i], SIGUSR2);

            sleep(1);

            pipeName = getPipename(childrenPids[i]);

            pid_t tmp = (pid_t) childrenPids[i];

            int signalResult = kill(tmp, SIGUSR1);
            if (signalResult != 0) {
                printf("Signal error\n");
            };
            fd = open(pipeName, O_RDWR);

            while (fd < 0) {
                fd = open(pipeName, O_RDWR);
                printf("Error opening file: %s\n", strerror(errno));
            }

            write(fd, message, strlen(message) + 1);

            close(fd);


        }


    }
    waitingResponse = true;
    return status;

}


bool list(){

    int status = printf("Elenco dispositivi\n");
    printLimb(limbo);


    TODO:
    // ask every children info

    return status >= 0;


}





bool add(char device[MAXLEN], int * id){



    bool status = true;


    // fare ENUM di tipi?

    (*id)++;

    if(strcmp(device, "hub\n") == 0) {

        // do smth
    }

    else if(strcmp(device, "timer\n") == 0) {

        // do smth
    }

    else if(strcmp(device, "bulb\n") == 0) { // type 2

        limbDevice * tmp = (limbDevice *) malloc(sizeof(limbDevice));

        if (tmp != NULL){



            tmp->id = *id;
            tmp->fId = -1;
            tmp->type = 2;
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

            printf("Fatal: failed to add bulb");
            status = false;
        }




        }


        // do smth

    else if(strcmp(device, "window\n") == 0) {

        // do smth
    }
    else if(strcmp(device, "fridge\n") == 0) {

        // do smth
    }

    else {
        // device not recognized
        status = false;
        (*id)--;

    }

    return status;
}





bool tie(int idChild, int idParent){
    waitingResponse = true;

    bool status = true;

    limbDevice * tmp = exists(idChild, limbo);
    if (tmp == NULL){
        printf("Device with ID %d not found or already linked", idChild); // cosa fare se già linkata?
        return false;
    }


    // controlla che parent sia controller
    // parte in broadcast una richiesta
    // il processo sa che tipo è da argv[0]


    else if (idParent == 0){
        int childType = tmp->type;


        pid_t pidChild = spawn(childType, idChild);

        if (! removeFromLimb(idChild, limbo)) {
            status = false;
        }




    }

    return true;
}

bool switchLabel(char * id, char label, char position) {
    char message[MAXLEN];

    sprintf(message, "%s down 0 %c;%c", id, label, position);

    bool status = true; // manage status below ?

    int i;
    for (i = 0; i < MAXLEN; i++) {


        if (childrenPids[i] != -1) {

            kill((pid_t) childrenPids[i], SIGUSR2);
            char *pipeName = getPipename(childrenPids[0]);

            kill((pid_t) childrenPids[i], SIGUSR1);

            int fd = open(pipeName, O_RDWR);

            while (fd < 0) {
                fd = open(pipeName, O_RDWR);
                printf("Error opening file: %s\n", strerror(errno));
            }

            write(fd, message, strlen(message) + 1);

            close(fd);


        }


    }
    sleep(2);
    return status;
}

void handleSignal(int sig){

    if (sig == SIGUSR2){
        waitingResponse = false;
        return;
    }


    //signal(SIGUSR1, handleSignal);

    char tmp[MAXLEN];
    int i;
    for (i=0; i<MAXLEN; i++){
        tmp[i] = '\0';
    }
    fdIn = open(fifoIn, O_RDONLY | O_NONBLOCK); // open pipe - non-blocking

    read(fdIn, tmp, MAXLEN);



    char * message[MAXLEN];
    tokenizer(tmp, message, " ");


    idReceiver = atoi(message[0]);
    idSender = atoi(message[2]);


    if (idReceiver == id) { // messaggio per me
        char * commands[MAXLEN];
        tokenizer(message[3], commands, ";");


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


                    //strftime (activeTimeStr, MAXLEN, "%Y-%m-%d %H:%M:%S.000", (localtime (&activeTime)).);
                    printf ("   - Active Time: %dh %dm %ds\n", hours, minutes, seconds);

                    break;
            }

        }
    }
    waitingResponse = false;
    close(fdIn);
}







int main(int argc, char *argv[]) {

    init(&childrenPids);

    int id = 0;

    limbo = (limb *) malloc(sizeof(limb));
    limbo->head = NULL;
    limbo->tail = NULL;

    pid = getpid();

    fifoIn = getPipename(pid);
    mkfifo(fifoIn, 0777);

    //signal(SIGUSR1, handleSignal);
    struct sigaction psa1;
    psa1.sa_handler = handleSignal;
    sigaction(SIGUSR1, &psa1, NULL);
    sigaction(SIGUSR2, &psa1, NULL);



    //Input stuff
    char buffer[MAXLEN];


    int status = 1; //Contains output of various operations

    //Text input cycle
    while (1) {
        //printf(" waiting %d\n", waitingResponse);
        while (!waitingResponse) {

            printf(" > ");

            if (fgets(buffer, MAXLEN, stdin) != NULL) { // not sure if working
                char *tokens[MAXLEN];
                tokenizer(buffer, tokens, " ");


                if (strcmp(tokens[0], "list\n") == 0) {
                    status = list();
                } else if (strcmp(tokens[0], "add") == 0) {
                    //printf("%s\n", tokens[1]);
                    if (tokens[1] != NULL) {


                        status = add(tokens[1], &id);
                        if (!status) {
                            printf("Device not recognized\n");
                        } else {
                            printf("Added bulb with ID %d\n", id);
                        }


                    }

                } else if (strcmp(tokens[0], "link") == 0) {

                    if (tokens[1] != NULL &&
                        ((strcmp(tokens[2], "to") == 0) && tokens[3] != NULL)) { // check better if id is valid

                        status = tie(atoi(tokens[1]), atoi(tokens[3]));

                        //status = add(tokens[1], &id);
                        if (!status) {
                            printf("Device not recognized\n");
                        } else {
                            printf("Linked\n");
                        }


                    }

                } else if (strcmp(tokens[0], "switch") == 0) {
                    if (tokens[1] != NULL && atoi(tokens[1]) > 0 && tokens[2] != NULL &&
                        tokens[3] != NULL) { // check better if id is valid

                        char *id = tokens[1];

                        char *labelStr = tokens[2];
                        char label;

                        char *positionStr = tokens[3];
                        char position;

                        // label check

                        if (strcmp(labelStr, "status") == 0) { // status bulb
                            label = STATUS_S;

                        } else {
                            printf("Label %s does not exist\n", tokens[2]);
                            break;
                        }


                        // position check

                        if (strcmp(positionStr, "on\n") == 0) {
                            position = ON_S;

                        } else if (strcmp(positionStr, "off\n") == 0) {
                            position = OFF_S;

                        } else {
                            printf("Position %s does not exist\n", tokens[3]);
                            break;
                        }


                        // all right

                        status = switchLabel(id, label, position);

                        if (!status) {
                            printf("Switch error\n");
                        }

                    }

                } else if (strcmp(tokens[0], "info") == 0) {
                    if (tokens[1] != NULL) {
                        status = info(tokens[1]);

                        if (!status) {
                            printf("info error\n");
                        }
                    }

                }


            } else {
                printf("Error reading from stdin!\n");
            }


        }

    }
    return 0;
}
