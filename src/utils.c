//
// Created by Alessia Marcolini on 2019-05-13.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

char * getPipename(long pid) {
    char * pipeName = malloc(4 * sizeof(pipeName));
    sprintf(pipeName, "/tmp/pipes/%ld", pid);
    return pipeName;
}

void tokenizer(char * buffer, char ** tokens, char * delimiter){


    // split
    char * delim = delimiter;

    char *ptr = strtok(buffer, delim);
    tokens[0] = ptr;

    int tokenIndex = 1;

    while(ptr != NULL){
        //printf("'%s'\n", ptr);
        ptr = strtok(NULL, delim);
        tokens[tokenIndex] = ptr;
        tokenIndex++;
    }
}

int calculateNewFreePosition(pid_t children_pids[MAXLEN], int lastPosition){
    int i;
    for (i=lastPosition; i<MAXLEN; i++){
        if (children_pids[i] == (pid_t) NULL){
            return i;

        }
    }
    return -1;

}


