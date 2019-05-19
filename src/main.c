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

#include "utils.h"
#include "limb.h"

#define debug



// structure containing children's pid
pid_t children_pids[MAXLEN]; // you can now calculate pipe names
int firstFreePosition = 0; // of the children's pid array - useful for inserting the next one without scanning all the array

void init(){
    // init children structure
    for (int i=0; i<MAXLEN; i++){
        children_pids[i] = (pid_t) NULL;
    }
}

void printChildren(){
    for (int i=0; i<MAXLEN; i++){
        if (children_pids[i] != (pid_t) NULL) {
            printf("%ld\n", (long) children_pids[i]);
        }
    }
}



void spawn(int type, int id) {

    pid_t pid = fork();

    if (pid == 0) { // child

        pid_t ppid = getppid();
        char *pipeName = getPipename(pid); // open parent-child pipe
        mkfifo(pipeName, O_RDWR);

        char deviceStr[MAXLEN];
        sprintf(deviceStr, "%d", id);
        char * const paramList[] = {"./bin/bulb", deviceStr, NULL}; // type 2
        execv(paramList[0], paramList);


    }
    else {
        children_pids[firstFreePosition] = pid;
        firstFreePosition = calculateNewFreePosition(children_pids, firstFreePosition);
        if (firstFreePosition == -1){
            printf("Ok but no room for other children");
        }
    }
}


bool list(limb * limbo){

    int status = printf("Elenco dispositivi\n");
    printLimb(limbo);
    return status >= 0;
}





bool add(char device[MAXLEN], int * id,  limb * limbo){



    bool status = true;
#ifdef debug
    printf("%s \n", device);
#endif


    // fare ENUM di tipi?

    (*id)++;

    if(strcmp(device, "hub\n") == 0) {

        // do smth
    }

    else if(strcmp(device, "timer\n") == 0) {

        // do smth
    }

    else if(strcmp(device, "bulb\n") == 0) { // type 2

        printf("ciao");
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





bool tie(int idChild, int idParent, limb * limbo){

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

        pid_t * pidChild;
        spawn(childType, idChild);

        if (! removeFromLimb(idChild, limbo)) {
            status = false;
        }


        printChildren();



    }

    return true;
}





int main(int argc, char *argv[]) {

    init();

    int id = 0;
    limb * limbo = (limb *) malloc(sizeof(limb));
    limbo->head = NULL;
    limbo->tail = NULL;


    //Input stuff
    char buffer[MAXLEN];


    int status = 1; //Contains output of various operations

    //Text input cycle
    while (1) {

        printf(" > ");

        if (fgets(buffer, MAXLEN, stdin) != NULL) {
            char * tokens[MAXLEN];
            tokenizer(buffer, tokens, " ");




            if (strcmp(tokens[0], "list\n")==0) {
                status = list(limbo);
            }

            else if (strcmp(tokens[0], "add")==0) {
                //printf("%s\n", tokens[1]);
                if (tokens[1] != NULL){



                    status = add(tokens[1], &id, limbo);
                    if (! status){
                        printf("Device not recognized\n");
                    }
                    else {
                        printf("Added bulb with ID %d\n", id);
                    }


                }

            }


            else if (strcmp(tokens[0], "link")==0) {

                if (tokens[1] != NULL && ((strcmp(tokens[2], "to") == 0) && tokens[3] != NULL)){

                    status = tie(atoi(tokens[1]), atoi(tokens[3]), limbo);

                    //status = add(tokens[1], &id);
                    if (! status){
                        printf("Device not recognized\n");
                    }
                    else {
                        printf("OK");
                    }


                }

            }
        } else {
            printf("Error reading from stdin!\n");
        }


    }
    return 0;
}