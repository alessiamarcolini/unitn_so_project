//
// Created by Alessia Marcolini on 2019-05-18.
//

#ifndef LIMB_H
#define LIMB_H

#endif

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


bool isLimbEmpty(limb * limbo);

limbDevice * existsRec(int idChild, limbDevice * head, limb * limbo);
limbDevice * exists(int idChild, limb * limbo);

bool removeFromLimb(int id, limb * limbo);

void printLimbRec(limb * limbo, limbDevice * head);
void printLimb(limb * limbo);