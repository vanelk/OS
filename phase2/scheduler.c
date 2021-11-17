#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/initial.h"


/*
The scheduling method. This method is called whenever
a new process is needed. The scheduling algorithm used
is round-robin.
	Parameters: NULL
	Return: NULL
*/
void scheduler(){
    cpu_t timeElapsed;
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
        /* update startTOD for current process */
        STCK(startTOD);
        /* set timer */
        setTIMER(QUANTUM);
        loadState(&(currentProc->p_s));
    } else {
        /* if there is no process in the ready queue, set currentproc to null */
        if(processCount == 0){
            HALT();
        }
        if(processCount > 0){
            /* if there are still processes running and they are blocked */
            if (softBlockCount > 0){
                /* mask to turn interrupts on */
                int mask = ALLOFF | IECON | IMON;
                /* set the status to the mask */
                setSTATUS(mask);
                /* we wait with Interrupts on */
                WAIT();
            }
            /* We have processes running but they are not blocked nor in the ready queue */ 
            if (softBlockCount == 0) PANIC();

        }
    }
}
/*
LDST container method because LDST is
TOO POWERFUL
	Parameter: ps -> process to load
	Return: NULL
*/
void loadState(state_PTR ps){
    LDST(ps);
}
