#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"
#include "../h/interrupts.h"

int swapSem;

void pager(){
    pcb_PTR currentProc;

    int cause = currentProc->s_sup_exceptState[0].cause;

    if(cause == 1){
	SYSCALL(SYSCALL2,ZERO,ZERO,ZERO);
    }else{
	SYSCALL(SYSCALL3,swapSem,ZERO,ZERO);
	int frame = pickVictim();
	SYSCALL(SYSCALL8,ZERO,ZERO,ZERO);
	currentProc->s_
    }
}

void uTLB_RefillHandler(){

}

int pickVictim(){
    static int i=0;
    i=(i+1)%POOLSIZE;
    return i;
}
