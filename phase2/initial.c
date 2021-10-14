#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"


/* Global variables */
int processCount; /* number of processes on the readyQueue */
int softBlockCount; /* number of processes waiting for IO */
pcb_PTR readyQueue; /* pointer to the ready queue */
pcb_PTR currentProc; /* pointer to the current process*/
int semDevices[DEVNUM]; /* 1 for each device available */

extern void test();
int main(){
    /* populate processor 0 passup vector. */
    passupvector_t* nuke = (passupvector_t *) (0x0FFFF900);
    nuke->tlb_refll_handler = (memaddr) uTLB_RefillHandler; /* to be  implemented */
    /* setting the stack pointer for the nucleus TBL-refill event handler to top of nucleus stack page */
    nuke->exception_stackPtr = 0x20001000; 
    nuke->exception_handler = (memaddr) SYSCALL;
    // load interupthandler
    nuke->exception_stackPtr = 0x20001000;
    /*initialize PCB and ASL*/
    initPcbs();
    initASL();

    /* set globals */
    processCount = 0;
    softBlockCount = 0;
    readyQueue = mkEmptyProcQ();
    currentProc = NULL;
    
    LDIT(QUANTUM);
    LDST(currentProc->p_s);

    scheduler();
}

void uTLB_RefillHandler(){
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST((state_PTR) 0x0FFFF000);
}