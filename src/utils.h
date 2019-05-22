//
// Created by Alessia Marcolini on 2019-05-13.
//

#ifndef UTILS_H
#define UTILS_H

#endif

// device type
#define CONTROLLER 0
#define BULB 1
#define FRIDGE 2
#define WINDOW 3
#define HUB 4
#define TIMER 5

// bulb stuff

// label
#define STATUS 0
#define STATUS_S '0'

#define INFO 1
#define INFO_S '1'

#define INFO_BACK 2
#define INFO_BACK_S '2'

// position
#define OFF 0
#define OFF_S '0'

#define ON 1
#define ON_S '1'




#define MAXLEN 256

char * getPipename(long pid);
int calculateNewFreePosition(long children_pids[MAXLEN], int lastPosition);
void tokenizer(char * buffer, char ** tokens, char * delimiter);
