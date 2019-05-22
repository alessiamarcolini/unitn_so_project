//
// Created by Alessia Marcolini on 2019-05-18.
//

#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#include "limb.h"
#include "utils.h"


bool isLimbEmpty(limb * limbo){
    return (limbo->head == NULL);
}


limbDevice * existsRec(int idChild, limbDevice * head, limb * limbo){
    if (head == NULL){
        return NULL;
    }
    if (head->id == idChild){
        return head;
    }

    else{
        return existsRec(idChild, head->next, limbo);
    }
}

limbDevice * exists(int idChild, limb * limbo){
    return existsRec(idChild, limbo->head, limbo);
}

bool removeFromLimb(int id, limb * limbo){
    bool status = true;
    limbDevice * tmp = exists(id, limbo);

    if (tmp != NULL){
        if ((limbo->head == limbo->tail) && (limbo->head == tmp)){
            limbo->head = NULL;
            limbo->tail = NULL;
        }
        else if (limbo->head == tmp){
            limbo->head = tmp->next;
        }
        else {
            limbDevice * tmp_head = limbo->head;
            while(tmp_head->next != tmp){
                tmp_head = tmp_head->next;
            }
            if (limbo->tail == tmp) {
                limbo->tail = tmp_head;
                limbo->tail->next = NULL;
            }


            else {
                tmp_head->next = tmp->next;
            }

        }

        free(tmp);

    }
    status = false;
    return status;
}


void printLimbRec(limb * limbo, limbDevice * head){
    if (head != NULL){
        if (head->fId == -1){
            printf("%d %d\n", head->type, head->id);
        }

        printLimbRec(limbo, head->next);
    }
}

void printLimb(limb * limbo){
    printLimbRec(limbo, limbo->head);
}
