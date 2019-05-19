//
// Created by Alessia Marcolini on 2019-05-13.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

char * getPipename(pid_t pid) {
    char * pipeName = malloc(4 * sizeof(char));
    sprintf(pipeName, "/tmp/pipes/%i", pid);
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
    for (int i=lastPosition; i<MAXLEN; i++){
        if (children_pids[i] == (pid_t) NULL){
            return i;

        }
    }
    return -1;

}

/*

pid_t getIdFromPid(pid_t pid) {
    char *pipeName = getPipename(pid);

    char tmpOut[MAXLEN];
    sprintf(tmpOut, , " ", (intmax_t) getpid(), " down", (intmax_t) pid, " getId");

    int fdDown = open(pipeName, O_WRONLY);
    if (fdDown < 0) {
        return NULL;
    }
    write(fdDown, tmpOut, strlen(tmp));

    if (kill(pid, SIGUSR1) < 0) {
        return NULL;
    }
    close(fdDown);

    int fdIn = open(getPipename(getPid()), O_RDONLY);

    if (fdIn < 0) {
        return NULL;
    }

    char tmpIn[MAXLEN];

    read(fdIn, tmpIN, MAXLEN);

    char **commands;

    tokenizer(tmpIn, commands, " ");
    */

