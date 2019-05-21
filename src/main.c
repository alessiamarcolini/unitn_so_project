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
#include <errno.h>

#include "utils.h"
#include "limb.h"

#define debug


limb * limbo;

// structure containing children's pid
long children_pids[MAXLEN]; // you can now calculate pipe names
int firstFreePosition = 0; // of the children's pid array - useful for inserting the next one without scanning all the array

void init(){
    // init children structure
    for (int i=0; i<MAXLEN; i++){
        children_pids[i] = -1;
    }
}

void printChildren(){
    for (int i=0; i<MAXLEN; i++){
        if (children_pids[i] != -1) {
            printf("index: %d, pid: %ld\n",i, children_pids[i]);
        }
    }
}



void spawn(int type, int id) {

    pid_t pid = fork();

    if (pid == 0) { // child

        char *pipeName = getPipename(getpid());
        mkfifo(pipeName, 0777);

        char deviceStr[MAXLEN];
        sprintf(deviceStr, "%d", id);
        char * const paramList[] = {"./bin/bulb", deviceStr, NULL}; // type 2
        int e = execv(paramList[0], paramList);
        if (e < 0){
            printf( "Error execv: %s\n", strerror( errno ) );
        }


    }
    else {
        children_pids[firstFreePosition] = (long) pid;
        firstFreePosition = calculateNewFreePosition(children_pids, firstFreePosition);

        if (firstFreePosition == -1){
            printf("Ok but no room for other children");

        }


    }
}


bool list(){

    int status = printf("Elenco dispositivi\n");
    printLimb(limbo);
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
    }

    return status;
}





bool tie(int idChild, int idParent){

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
        printf("Linking to centralina, device n: %d\n", idChild);
        int childType = tmp->type;


        spawn(childType, idChild);

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


    for (int i = 0; i < MAXLEN; i++) {


        if (children_pids[i] != -1) {
            char *pipeName = getPipename(children_pids[0]);

            pid_t tmp = (pid_t) children_pids[i];
            kill(tmp, SIGUSR1);

            int fd = open(pipeName, O_RDWR);

            while (fd < 0) {
                fd = open(pipeName, O_RDWR);
                printf("Error opening file: %s\n", strerror(errno));
            }

            write(fd, message, strlen(message) + 1);

            close(fd);


        }


    }

    return status;
}





int main(int argc, char *argv[]) {

    init();

    int id = 0;
    limbo = (limb *) malloc(sizeof(limb));
    limbo->head = NULL;
    limbo->tail = NULL;


    //Input stuff
    char buffer[MAXLEN];


    int status = 1; //Contains output of various operations

    //Text input cycle
    while (1) {

        printf(" > ");

        if (fgets(buffer, MAXLEN, stdin) != NULL) { // not sure if working
            char * tokens[MAXLEN];
            tokenizer(buffer, tokens, " ");


            if (strcmp(tokens[0], "list\n")==0) {
                status = list();
            }

            else if (strcmp(tokens[0], "add")==0) {
                //printf("%s\n", tokens[1]);
                if (tokens[1] != NULL){



                    status = add(tokens[1], &id);
                    if (! status){
                        printf("Device not recognized\n");
                    }
                    else {
                        printf("Added bulb with ID %d\n", id);
                    }


                }

            }


            else if (strcmp(tokens[0], "link")==0) {

                if (tokens[1] != NULL && ((strcmp(tokens[2], "to") == 0) && tokens[3] != NULL)){ // check better if id is valid

                    status = tie(atoi(tokens[1]), atoi(tokens[3]));

                    //status = add(tokens[1], &id);
                    if (! status){
                        printf("Device not recognized\n");
                    }
                    else {
                        printf("OK");
                    }


                }

            }


            else if (strcmp(tokens[0], "switch")==0) {
                if (tokens[1] != NULL && atoi(tokens[1]) > 0 && tokens[2] != NULL && tokens[3] != NULL){ // check better if id is valid

                    char * id = tokens[1];

                    char * labelStr = tokens[2];
                    char label;

                    char * positionStr = tokens[3];
                    char position;

                    // label check

                    if (strcmp(labelStr, "status") == 0){ // status bulb
                        label = STATUS_S;

                    }

                    else {
                        printf("Label %s does not exist\n", tokens[2]);
                        break;
                    }


                    // position check

                    if (strcmp(positionStr, "on\n") == 0){
                        position = ON_S;

                    }

                    else if (strcmp(positionStr, "off\n") == 0){
                        position = OFF_S;

                    }

                    else{
                        printf("Position %s does not exist\n", tokens[3]);
                        break;
                    }


                    // all right

                    status = switchLabel(id, label, position);

                    if (!status){
                        printf("Switch error");
                    }

                }

            }


        } else {
            printf("Error reading from stdin!\n");
        }


    }
    return 0;
}