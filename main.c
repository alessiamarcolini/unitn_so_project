//
// Created by Alessia Marcolini on 2019-04-29.
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

const int MAXLEN = 256;

int main(int argc, char *argv[]) {


    char buffer[MAXLEN];
    char * tokens[MAXLEN];


    while (1) {

        printf(" > ");

        if (fgets(buffer, 256, stdin) != NULL) {

            // split
            char delim[] = " ";

            char *ptr = strtok(buffer, delim);
            tokens[0] = ptr;



            int tokenIndex = 1;

            while(ptr != NULL)
            {
                printf("'%s'\n", ptr);
                ptr = strtok(NULL, delim);
                tokens[tokenIndex] = ptr;
            }

            


            if (strcmp(tokens[0], "list\n")==0) {
                printf("list\n");
            }

            if (strcmp(tokens[0], "add")==0) {
                printf("add\n");
            }
        } else {
            printf("Error reading from stdin!\n");
        }


    }
    return 0;
}