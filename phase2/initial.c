#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/interrupts.h"

/**************************************************************************************************************/
/*                                            INITIAL.c                                                       */
/* This module contains the main function for the OS. This is the nucleus of the code where we initialize     */
/* pcbs and semaphores from phase 1 and initialize the memory adresses. Finally it begins the first process   */
/* and then calls the scheduler. This module also contains the main method for exception handling.            */
/**************************************************************************************************************/

/* Global variables */
int processCount; /* number of processes of created processes still active */
int softBlockCount; /* number of processes waiting for IO */
pcb_PTR readyQueue; /* pointer to the ready queue */
pcb_PTR currentProc; /* pointer to the current process*/
int semDevices[DEVNUM]; /* 1 for each device available (49) in total */
cpu_t startTOD;
int *clockSem = &semDevices[DEVNUM-ONE];

/*Helper Methods*/
extern void test();
extern void uTLB_RefillHandler();
HIDDEN void exceptionHandler();

/*
The main function of the OS. initialize pcbs and semaphores. set memory adresses.
starts first process and calls scheduler. only runs at the beginning.
*/
int main(){
    /*initialize PCB and ASL*/
    initPcbs();
    initASL();

    devregarea_t* devBus = (devregarea_t*) RAMBASEADDR;
    int TopOfRAM = (devBus->rambase + devBus->ramsize); /*set top of ram memory address*/
    /* populate processor 0 passup vector. */
    passupvector_t* nuke = (passupvector_t *) PASSUPVECTOR;
    nuke->tlb_refll_handler = (memaddr) uTLB_RefillHandler;
    /* setting the stack pointer for the nucleus TBL-refill event handler to top of nucleus stack page */
    nuke->tlb_refll_stackPtr = NUKE; 
    nuke->exception_handler = (memaddr) exceptionHandler;
    /* load interupthandler */
    nuke->exception_stackPtr = NUKE;

    /* set globals */
    processCount = 0;
    softBlockCount = 0;
    readyQueue = mkEmptyProcQ();
    currentProc = NULL;
    /* initialize the device semaphores' value to 0 */
    int i;
    for(i=0; i<DEVNUM; i++)  semDevices[i] = ZERO;
    /* allocate the first process */
    pcb_PTR firstProc = allocPcb();
    /* if the allocation failed, which should not happen we PANIC */
    if(firstProc == NULL){
        PANIC();
    }else{
        /* set current process register values */
        firstProc->p_s.s_pc = firstProc->p_s.s_t9 = (memaddr) test;
        firstProc->p_s.s_status = ALLOFF | IEON | IMON;
        firstProc->p_s.s_sp = TopOfRAM;
        firstProc->p_supportStruct = NULL;
        /* insert the current process in to the ready queue */
        insertProcQ(&readyQueue, firstProc);
        processCount++;
        LDIT(IOCLOCK);
        firstProc = NULL;
        /* set startTOD */
        STCK(startTOD);
        scheduler();
    }
    return 0;
    
}

/*
Method to handle all exceptions and send them where they are
needed. if the reason is 0 then exception is an IO,
if its <= 7 it is a TLB or GENERAL exception that will be passed up
and if its an 8 then it is a SYSCALL.
	Parameters: NULL
	Return: NULL
*/
void exceptionHandler(){
    state_PTR oldstate;
    oldstate = (state_PTR) BIOSDATAPAGE;
    /* get the reason for exception from cause register */
    int reason = ((oldstate->s_cause & EXCODEMASK) >> 2);
    /* if the reason is 0 then exception is an IO interrupt*/
    if(reason == 0) IOHandler();
    /* either a general exception or a page fault exception */
    if(reason <= 7 || reason > 8) otherExceptions(reason);
    /* if the reason is 8 then it is a SYSCALL */
    if(reason == 8) SYSCALLHandler();

}
