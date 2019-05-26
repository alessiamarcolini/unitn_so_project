#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#include "limb.h"
#include "utils.h"

//limb is a data structure containing all devices that were added but not linked to the system
//it does not contain any actual living process, but it temporarily contains basic information about "suspended" devices
//it is also needed as a temporary place to store the tree before printing it during a list operation
//it can also contain the description of a subtree while moving a device which is not an end device.

bool isLimbEmpty(limb * limbo){
    return (limbo->head == NULL);
}

//recursive part of the "exists" function
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

//checks if an id belongs to the limb
//if it does then it returns a pointer to the device itself
//wrapper method
limbDevice * exists(int idChild, limb * limbo){
    return existsRec(idChild, limbo->head, limbo);
}

//removes an item from the limb given its id
//returns true if the removal was successful
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
    else {
        status = false;
    }

    return status;
}

//recursive part of the "printLimb" method
void printLimbRec(limb * limbo, limbDevice * head){
    if (head != NULL){
        if (head->fId == -1){
            printf("Device type: %d, id: %d\n", head->type, head->id);
        }

        printLimbRec(limbo, head->next);
    }
}
//prints the whole limb
//wrapper method
void printLimb(limb * limbo){
    printLimbRec(limbo, limbo->head);
}
