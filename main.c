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

#define debug 0

const int MAXLEN = 256;

void removeNewline(char device[MAXLEN]){
    int len = strlen(device);
    //printf("%d", len);
    char tmp[len-1];
    for (int i=0; i<len-1; i++){
        tmp[i] = device[i];
    }

    tmp[len-1] = device[len]; // add \0


    device = tmp;
#ifdef debug
    printf("%s \n", "quello di nl");
    printf("%s \n", device);
#endif
}


bool list(){

    int status = printf("Elenco dispositivi\n");
    return status >= 0;
}

bool add(char device[MAXLEN]){
#ifdef debug
    printf("%s \n", device);
#endif
    removeNewline(device);
    bool status = true;
#ifdef debug
    printf("%s \n", device);
#endif

    if(strcmp(device, "hub") == 0) {

        // do smth
    }

    else if(strcmp(device, "timer") == 0) {

        // do smth
    }

    else if(strcmp(device, "bulb") == 0) {

        // do smth
    }
    else if(strcmp(device, "window") == 0) {

        // do smth
    }
    else if(strcmp(device, "fridge") == 0) {

        // do smth
    }

    else {
        // device not recognized
        status = false;
    }

    return status;
}




int main(int argc, char *argv[]) {


    char buffer[MAXLEN];
    char * tokens[MAXLEN];

    int status = 1; //Contains output of various operations

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

            if (strcmp(tokens[0], "add")==0) {
                //printf("%s\n", tokens[1]);
                if (tokens[1] != NULL){
                    status = add(tokens[1]);
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