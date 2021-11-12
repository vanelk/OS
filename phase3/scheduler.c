#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/initial.h"

cpu_t timeElapsed;

/* The scheduling algorithm implemented is round-robin*/
void scheduler(){
    if(currentProc!=NULL){
        /* get time how long the process has been running*/
        STCK(timeElapsed);
        /* Set time of current cpu */
        currentProc->p_time = currentProc->p_time + (timeElapsed - startTOD);
        LDIT(IOCLOCK);
    }
    /* remove next process from the ready queue */
    pcb_PTR next = removeProcQ(&readyQueue);
    /* check if the proccess exists */
    
    if (next != NULL){
        /* set currentproc to the next process */
        currentProc = next;
        STCK(startTOD);
        setTIMER(QUANTUM);
        LDST(&(currentProc->p_s));
    } else {
        /* if there is no process in the ready queue, set currentproc to null */
        if(processCount == 0){
            HALT();
        }
        if(processCount > 0){
            if (softBlockCount > 0){
                /* we wait with Interrupts and exceptions on */
                int mask = ALLOFF | IECON | IMON;
                setSTATUS(mask);
                WAIT();
            } 
            if (softBlockCount == 0) PANIC();

        }
    }
}

void loadState(state_PTR ps){
    LDST(ps);
}
