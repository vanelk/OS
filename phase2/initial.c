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
int semDevices[DEVNUM]; /* 1 for each device available (49) in total */
cpu_t startTOD;
extern void test();



int main(){
    /*initialize PCB and ASL*/
    initPcbs();
    initASL();

    devregarea_t* devBus = (devregarea_t*) RAMBASEADDR;
    int TopOfRAM = (devBus->rambase + devBus->ramsize); /*set top of ram memory address*/
    /* populate processor 0 passup vector. */
    passupvector_t* nuke = (passupvector_t *) (0x0FFFF900);
    nuke->tlb_refll_handler = (memaddr) uTLB_RefillHandler; /* to be  implemented */
    /* setting the stack pointer for the nucleus TBL-refill event handler to top of nucleus stack page */
    nuke->exception_stackPtr = 0x20001000; 
    nuke->exception_handler = (memaddr) exceptionHandler;
    /* load interupthandler */
    nuke->exception_stackPtr = 0x20001000;

    /* set globals */
    processCount = 0;
    softBlockCount = 0;
    readyQueue = mkEmptyProcQ();
    currentProc = allocPcb();
    if(currentProc!= NULL){
        currentProc->p_s.s_pc = currentProc->p_s.s_t9 = (memaddr) test;
        currentProc->p_s.s_status = 100000;
        currentProc->p_s.s_sp = (TopOfRAM - PAGESIZE);
        currentProc->p_supportStruct = NULL;
        insertProcQ(&readyQueue, currentProc);
        processCount++;
        LDIT(startTOD);

        scheduler();

    } else {
        PANIC();
    }
    return 0;
    
}

void uTLB_RefillHandler(){
    setENTRYHI(0x80000000);
    setENTRYLO(0x00000000);
    TLBWR();
    LDST((state_PTR) 0x0FFFF000);
}

exceptionHandler(){
    state_PTR oldstate;
    oldstate = (state_PTR) BIOSDATAPAGE;
    int reason = (oldstate->s_cause >> 2) << 25;
    if(reason == 0) interuptIOTrap();
    if(reason == 8) SYSCALL();

}