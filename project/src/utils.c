#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

//this library contains methods that are useful for most devices in the system

//string sanitization function
void removeNewLine(char * str){
    char *p = strchr(str, '\n');  // finds first, if any, \n
    if (p != NULL)
        *p = '\0';
}

//returns a pipename given a pid
char * getPipename(long pid) {
    char * pipeName = malloc(4 * sizeof(pipeName));
    sprintf(pipeName, "/tmp/pipes/%ld", pid);
    return pipeName;
}

//tokenizes a string into tokens delimited by a given delimiter char
void tokenizer(char * buffer, char ** tokens, char * delimiter){

    // split
    char * delim = delimiter;

    char *ptr;
    int tokenIndex;
    ptr = strtok(buffer, delim);
    tokens[0] = ptr;

    tokenIndex = 1;

    while(ptr != NULL){
        //printf("'%s'\n", ptr);
        ptr = strtok(NULL, delim);
        tokens[tokenIndex] = ptr;
        tokenIndex++;
    }
}

//find first available position (if any) in a given array of pids and returns it
int calculateNewFreePosition(long children_pids[MAXLEN], int lastPosition){
    int i;
    for (i=lastPosition; i<MAXLEN; i++){
        if (children_pids[i] == -1){
            return i;

        }
    }
    return -1;

}

//initializes all positions of children pids array to -1
void initChildren(long childrenPids[MAXLEN]){
    // init children structure
    int i;
    for (i=0; i<MAXLEN; i++){
        childrenPids[i] = -1;
    }
}

//prints an array of pids (only the valorized ones)
void printChildren(long childrenPids[MAXLEN]){
    int i;
    for (i=0; i<MAXLEN; i++){
        if (childrenPids[i] != -1) {
            printf("index: %d, pid: %ld\n",i, childrenPids[i]);
        }
    }
}


