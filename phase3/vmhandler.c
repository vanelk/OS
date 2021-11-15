#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"
#include "../h/interrupts.h"

extern pcb_PTR currentProc;
extern swapPool [POOLSIZE];
void pager(){
    
    support_t * support = SYSCALL(SYSCALL8, ZERO, ZERO, ZERO);
    state_t exceptionState = support->sup_exceptState[PGFAULTEXCEPT];
    int cause = exceptionState.s_cause;
    // If cause isa TLB Modification exception, threat it as a program trap
    if(cause == 1){
	    SYSCALL(SYSCALL2, ZERO, ZERO, ZERO);
    }else{
        /* mutual exclusion of the swap pool table */
        SYSCALL(SYSCALL3, swapSem, ZERO, ZERO);
        int p = exceptionState.s_entryHI;
        int frame = pickVictim();
        /* selected frame is used */
        if(swapPool[frame].asid!=-1){
            
        }
        
    }
}

void uTLB_RefillHandler(){
    state_PTR bios = (state_PTR) BIOSDATAPAGE;
    setENTRYHI(currentProc->p_s.s_entryHI);
    setENTRYLO(currentProc->p_s.s_entryLO);
    TLBWR();
    LDST(bios);


}

int pickVictim(){
    static int i=0;
    i=(i+1)%POOLSIZE;
    return i;
}