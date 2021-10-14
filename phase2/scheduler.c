#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
extern currentProc;
extern readyQueue;
extern processCount;
extern softBlockCount;
void scheduler(){
    pcb_PTR next = removeProcQ(&readyQueue);
    if (next != NULL){
        currentProc = next;
        //currentProc->cpu_t = 
        // store clock time elapsed
        // set timer
        // LDST
    }
    if(processCount == 0){
        HALT();
    }
    if(processCount > 0 && softBlockCount > 0){
        WAIT();
    }
    if (processCount > 0 && softBlockCount == 0){
        PANIC();
    }
}