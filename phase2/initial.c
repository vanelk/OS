#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/interrupts.h"

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

void debug(int a, int b){
    int i = 2+4;
}
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
    currentProc = allocPcb();
    int i;
    for(i=0; i<DEVNUM; i++)  semDevices[i] = ZERO;
    if(currentProc!= NULL){
        currentProc->p_s.s_pc = currentProc->p_s.s_t9 = (memaddr) test;
        currentProc->p_s.s_status = ALLOFF | IEON | IMON | TEBITON;
        currentProc->p_s.s_sp = TopOfRAM;
        currentProc->p_supportStruct = NULL;
        insertProcQ(&readyQueue, currentProc);
        processCount++;
        LDIT(IOCLOCK);
        currentProc = NULL;
        STCK(startTOD);
        scheduler();

    } else {
        PANIC();
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
    int reason = ((oldstate->s_cause & 0x0000007c) >> 2);
    if(reason == 0) IOHandler();
    if(reason <= 7 || reason > 8) otherExceptions(reason);
    if(reason == 8) SYSCALLHandler();

}
