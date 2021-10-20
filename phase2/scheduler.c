#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
extern currentProc;
extern readyQueue;
extern processCount;
extern softBlockCount;
extern startTOD;
cpu_t timeElapsed;
/* The scheduling algorithm implemented is round-robin*/
void scheduler(){
    /* get time how long the process has been running*/
    STCK(timeElapsed);
    /* Set time of current cpu */
    currentProc->p_time = currentProc->p_time + (timeElapsed - startTOD);
    /* remove next process from the ready queue */
    pcb_PTR next = removeProcQ(&readyQueue);
    /* check if the proccess exists */
    if (next != NULL){
        /* set currentproc to the next process */
        currentProc = next;
        LDIT(startTOD);
        loadState(&currentProc->p_s);
    }
    if(processCount == 0){
        HALT();
    }
    if(processCount > 0){
        if (softBlockCount > 0) WAIT();
        if(softBlockCount == 0) PANIC();

    }
}

void loadState(state_PTR ps){
    LDST(ps);
}