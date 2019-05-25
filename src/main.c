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
#include "controllerActions.h"

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
int numChildren = 0;


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

            printf("Fatal: failed to add bulb");
            status = false;
        }

    }

    else if(strcmp(device, "timer\n") == 0) {

        // do smth
    }

    else if(strcmp(device, "bulb\n") == 0) { // type 2

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





bool tie(char * idChild, char * idParent, bool * waitingResponse){
    *waitingResponse = true;

    bool status = true;

    limbDevice * tmp = exists(atoi(idChild), limbo);
    int childType;

    if (tmp == NULL){
        printf("Device with ID %s not found or already linked", idChild); // cosa fare se già linkata?
        status = false;
    }


    // controlla che parent sia controller
    // parte in broadcast una richiesta
    // il processo sa che tipo è da argv[0]



    else if (atoi(idParent) == 0){
        printf("link to 0\n");

        childType = tmp->type;
        pid_t pidChild = spawn(childType, idChild, childrenPids, &firstFreePosition);
        if (pidChild == -1){
            status = false;
            printf("Spawn error\n");
        }
        else {
            numChildren++;
            status = true;
        }

    }

    else { // forward

        childType = tmp->type;
        char message[MAXLEN];
        sprintf(message, "%d down 0 %d;%s;%d", atoi(idParent), SPAWN, idChild, childType);

        status = writeAllChildren(message, childrenPids);


    }
    if (! removeFromLimb(atoi(idChild), limbo)) {
        status = false;
        printf("error remove from limb\n");
    }

    return status;
}


void handleSignal(int sig){

    if (sig == SIGUSR2){
        waitingResponse = false;
        printf("ok sigusr2 main received\n");
        return;
    }


    //signal(SIGUSR1, handleSignal);

    char tmp[MAXLEN];
    int i;
    for (i=0; i<MAXLEN; i++){
        tmp[i] = '\0';
    }
    fdIn = open(fifoIn, O_RDWR); // open pipe

    while (fdIn < 0) {
        printf("Error opening pipe to bulb with id: %d and pid: %ld", id, (long) pid);
        fdIn = open(fifoIn, O_RDWR);
    }

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

    initChildren(childrenPids);

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
    psa1.sa_flags = SA_ONSTACK;
    sigaction(SIGUSR1, &psa1, NULL);
    sigaction(SIGUSR2, &psa1, NULL);

    static char stack[SIGSTKSZ];
    stack_t ss = {
            .ss_size = SIGSTKSZ,
            .ss_sp = stack,
    };
    sigaltstack(&ss, 0);



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
                            printf("Added new device with id %d of type %s", id, tokens[1]);
                        }


                    }

                } else if (strcmp(tokens[0], "link") == 0) {

                    if (tokens[1] != NULL &&
                        ((strcmp(tokens[2], "to") == 0) && tokens[3] != NULL)) { // check better if id is valid

                        status = tie(tokens[1], tokens[3], &waitingResponse);

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
