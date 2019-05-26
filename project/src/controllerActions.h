//
// Created by Alessia Marcolini on 2019-05-23.
//

#ifndef CONTROLLERACTIONS_H
#define CONTROLLERACTIONS_H

#endif


bool switchLabel(char * id, char label, char position, long childrenPids[MAXLEN]);

pid_t spawn(int type, char * id, long childrenPids[MAXLEN], int * firstFreePosition);

bool info(char * id, long childrenPids[MAXLEN], bool * waitingResponse);

bool writeAllChildren(char message[MAXLEN], long childrenPids[MAXLEN]);