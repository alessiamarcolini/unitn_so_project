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

#define debug

typedef struct limbDev {
    int id;
    int fId;    //father ID
    int type;
    char * registers;
    struct limbDev *next;
} limbDevice;

typedef struct limb {
    limbDevice *head;
    limbDevice *tail;
} limb;


bool isLimbEmpty(limb * limbo){
    return (limbo->head == NULL);
}



char * getPipename(int pid) {
    char * pipeName = malloc(4 * sizeof(char));
    sprintf(pipeName, "/tmp/ipc/%i", pid);
    return pipeName;
}

void printLimbRec(limb * limbo, limbDevice * head){
    if (head != NULL){
        if (head->fId == -1){
            printf("%d %d\n", head->type, head->id);
        }

        printLimbRec(limbo, head->next);
    }


}

void printLimb(limb * limbo){
    printLimbRec(limbo, limbo->head);
}





bool list(limb * limbo){

    int status = printf("Elenco dispositivi\n");
    printLimb(limbo);
    return status >= 0;
}





bool add(char device[MAXLEN], int * deviceIndex,  limb * limbo){



    bool status = true;
#ifdef debug
    printf("%s \n", device);
#endif


    // fare ENUM di tipi?

    (*deviceIndex)++;
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



            tmp->id = *deviceIndex;
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

        /*
         pid_t pid = fork();

        if (pid == 0){ // child

            char * pipeName = getPipename(getpid()); // open parent-child pipe
            mkfifo(pipeName, 0666);


            char *indexStr = malloc(4 * sizeof(char));
            sprintf(indexStr, "%d", *deviceIndex);




            char * const paramList[] = {"./bin/bulb", NULL};
            execv("./bin/bulb", paramList);
            */
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

limbDevice * exists1(int idChild, limbDevice * head, limb * limbo){
    if (head == NULL){
        return NULL;
    }
    if (head->id == idChild){
        return head;
    }

    else{
        return exists1(idChild, head->next, limbo);
    }
}

limbDevice * exists(int idChild, limb * limbo){
    return exists1(idChild, limbo->head, limbo);
}



bool tie(int idChild, int idParent, limb * limbo){

    limbDevice * tmp = exists(idChild, limbo);
    if (tmp == NULL){
        printf("Device with ID %d not found or already linked", idChild); // cosa fare se già linkata?
    }


    // controlla che parent sia controller
    // parte in broadcast una richiesta
    // il processo sa che tipo è da argv[0]

    return true;
}





int main(int argc, char *argv[]) {

    int deviceIndex = 0;
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
            tokenizer(buffer, tokens);




            if (strcmp(tokens[0], "list\n")==0) {
                status = list(limbo);
            }

            else if (strcmp(tokens[0], "add")==0) {
                printf("ciao add");
                //printf("%s\n", tokens[1]);
                if (tokens[1] != NULL){



                    status = add(tokens[1], &deviceIndex, limbo);
                    if (! status){
                        printf("Device not recognized\n");
                    }
                    else {
                        printf("Added bulb with ID %d\n", deviceIndex);
                    }


                }

            }


            else if (strcmp(tokens[0], "link")==0) {

                if (tokens[1] != NULL && ((strcmp(tokens[2], "to") == 0) && tokens[3] != NULL)){

                    status = tie(*tokens[1], *tokens[3], limbo);

                    //status = add(tokens[1], &deviceIndex);
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