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

#define debug 0

const int MAXLEN = 256;

typedef struct limbDev {
    int id;
    int fId;    //father ID
    int type;
    struct limbDev *next;
} limbDevice;

typedef struct limb {
    limbDevice *head;
    limbDevice *tail;
} limb;






char * getPipename(int pid) {
    char * pipeName = malloc(4 * sizeof(char));
    sprintf(pipeName, "/tmp/ipc/%i", pid);
    return pipeName;
}



bool list(){

    int status = printf("Elenco dispositivi\n");
    return status >= 0;
}





bool add(char device[MAXLEN], int * deviceIndex){

    (*deviceIndex)++;

    bool status = true;
#ifdef debug
    printf("%s \n", device);
#endif

    if(strcmp(device, "hub\n") == 0) {

        // do smth
    }

    else if(strcmp(device, "timer\n") == 0) {

        // do smth
    }

    else if(strcmp(device, "bulb\n") == 0) {

        pid_t pid = fork();

        if (pid == 0){ // child

            char * pipeName = getPipename(getpid()); // open parent-child pipe
            mkfifo(pipeName, 0666);


            char *indexStr = malloc(4 * sizeof(char));
            sprintf(indexStr, "%d", *deviceIndex);




            char * const paramList[] = {"./bin/bulb", NULL};
            execv("./bin/bulb", paramList);
        }


        // do smth
    }
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

    //dummy device
    limbDevice * tmp = (limbDevice *) malloc(sizeof(limbDevice));
    tmp->id = 1;

    limbo->head = tmp;


    limbDevice * tmp1 = (limbDevice *) malloc(sizeof(limbDevice));
    tmp1->id = 2;
    tmp->next = NULL;

    tmp->next = tmp1;

    //end dummy device



    // fake search
    // printf("%d", exists(4, limbo) != NULL);

    //Input stuff
    char buffer[MAXLEN];
    char * tokens[MAXLEN];

    int status = 1; //Contains output of various operations

    //Text input cycle
    while (1) {

        printf(" > ");

        if (fgets(buffer, MAXLEN, stdin) != NULL) {
            int init_size = strlen(buffer);
            // split
            char delim[] = " ";

            char *ptr = strtok(buffer, delim);
            tokens[0] = ptr;


            int tokenIndex = 1;


            while(ptr != NULL)
            {

                //printf("'%s'\n", ptr);
                ptr = strtok(NULL, delim);
                tokens[tokenIndex] = ptr;
                tokenIndex++;
            }


#ifdef debug
            for(int i = 0; i < tokenIndex; i++){
                printf("%s \n", tokens[i]);
            }
#endif

            if (strcmp(tokens[0], "list\n")==0) {
                status = list();
            }

            else if (strcmp(tokens[0], "add")==0) {
                //printf("%s\n", tokens[1]);
                if (tokens[1] != NULL){



                    status = add(tokens[1], &deviceIndex);
                    if (! status){
                        printf("Device not recognized\n");
                    }
                    else {
                        printf("OK");
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