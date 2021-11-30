#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"
#include "../h/interrupts.h"

extern int swapSem;
extern swap_t swapPool [POOLSIZE];

void pager(){
    support_t * support = SYSCALL(GETSUPPORTPTR, ZERO, ZERO, ZERO);
    state_t exceptionState = support->sup_exceptState[PGFAULTEXCEPT];
    int cause = exceptionState.s_cause;
    /* If cause isa TLB Modification exception, threat it as a program trap */
    if(cause == 1){
	    SYSCALL(TERMINATEPROCESS, ZERO, ZERO, ZERO);
    }else{
        /* mutual exclusion of the swap pool table */
        SYSCALL(PASSEREN, &swapSem, ZERO, ZERO);
        int missingPgNo = exceptionState.s_entryHI & GETPAGENO >> VPNSHIFT; // tbfout
        int frame = pickVictim();
        /* selected frame is used */
        pteEntry_PTR page;
        if(swapPool[frame].sw_asid!=-1){
            interruptsSwitch(0);
            TLBCLR();
            pteEntry_PTR page = (pteEntry_PTR) (0x20020000 + frame* PAGESIZE);
            page->entryLO &= ~(512);
            /* TODO: define this stuff */
            flashIO(1, page, swapPool[frame].sw_asid);
            /* might need to move up 1 line */
            interruptsSwitch(1);
        }
        /* TODO: define this stuff */
        flashIO(0, page, support->sup_asid);
        swapPool[frame].sw_asid = support->sup_asid;
        swapPool[frame].sw_pageNo = missingPgNo;
        swapPool[frame].sw_pte = page;
        SYSCALL(VERHOGEN, &swapSem, ZERO, ZERO);
        
    }
}
void uTLB_RefillHandler(){
    state_PTR bios = (state_PTR) BIOSDATAPAGE;
    setENTRYHI(currentProc->p_s.s_entryHI);
    setENTRYLO(currentProc->p_s.s_entryLO);
    TLBWR();
    LDST(bios);
}