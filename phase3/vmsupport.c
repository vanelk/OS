#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "../h/initproc.h"



void pager(){
    support_t * support = SYSCALL(GETSUPPORTPTR, ZERO, ZERO, ZERO);
    state_PTR exceptionState = &support->sup_exceptState[GENERALEXCEPT];
    int cause = exceptionState->s_cause;
    /* If cause isa TLB Modification exception, threat it as a program trap */
    if(cause == 1){
	    SYSCALL(TERMINATE, ZERO, ZERO, ZERO);
    }else{
        /* mutual exclusion of the swap pool table */
        SYSCALL(PASSEREN, &swapSem, ZERO, ZERO);
        int missingPgNo = exceptionState->s_entryHI & GETPAGENO >> VPNSHIFT; /* tbfout */
        int frame = pickVictim();
        /* selected frame is used */
        pteEntry_PTR page = (pteEntry_PTR) (0x20020000 + ((frame - 1)* PAGESIZE));
        if(swapPool[frame].sw_asid!=-1){
            interruptsSwitch(0);
            TLBCLR();
            page->entryLO &= ~(512);
            /* TODO: define this stuff */
            interruptsSwitch(1);
            flashIO(1, frame, page, swapPool[frame].sw_asid);
        }
        flashIO(0, missingPgNo, page, support->sup_asid-1);
        swapPool[frame].sw_asid = support->sup_asid;
        swapPool[frame].sw_pageNo = missingPgNo;
        swapPool[frame].sw_pte = page;
        TLBCLR();
        SYSCALL(VERHOGEN, &swapSem, ZERO, ZERO);
        LDST(exceptionState);  
    }
}
void uTLB_RefillHandler(){
    state_PTR bios = (state_PTR) BIOSDATAPAGE;
    setENTRYHI(bios->s_entryHI);
	setENTRYLO(getENTRYLO());
    /*setENTRYLO(currentProc->p_s.);*/
    TLBWR();
    LDST(bios);
}

int pickVictim(){
    static int i=0;
    i=(i+1)%POOLSIZE;
    return i;
}
void flashIO(int writeOn, int blockNum, memaddr data, int flashIOdNo){
    interruptsSwitch(0);
    devregarea_t * ram = RAMBASEADDR;
    
    device_t* flash = (&ram->devreg[8+flashIOdNo]); /*(device_t *) ((0x10000054 + 0x80) + (flashIOdNo * 0x10));*/
    /*SYSCALL(PASSEREN, (int)&devicesSem[8+flashIOdNo], 0, 0);*/				/* P(flash_mut) */
    flash->d_data0 = data;
    flash->d_command = (blockNum << 8) | 2+writeOn;
    interruptsSwitch(1);
    SYSCALL(WAITIO, FLASHINT, flashIOdNo, 0);
    /*SYSCALL(VERHOGEN, (int)&devicesSem[8+flashIOdNo], 0, 0);*/				/* V(flash_mut) */

}