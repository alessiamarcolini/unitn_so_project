//
// Created by Alessia Marcolini on 2019-05-13.
//

#ifndef UTILS_H
#define UTILS_H

#endif

#define CONTROLLER 0
#define BULB 1
#define FRIDGE 2
#define WINDOW 3
#define HUB 4
#define TIMER 5

// bulb stuff

#define STATUS 0
#define OFF 0
#define ON 1

#define STATUS_S '0'
#define OFF_S '0'
#define ON_S '1'

#define MAXLEN 256

char * getPipename(pid_t pid);

void tokenizer(char * buffer, char ** tokens, char * delimiter);
