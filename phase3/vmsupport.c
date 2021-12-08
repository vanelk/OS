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
    state_PTR exceptionState = &support->sup_exceptState[PGFAULTEXCEPT];
    int cause = exceptionState->s_cause;
    /* If cause isa TLB Modification exception, threat it as a program trap */
    if(cause == 1){
	    SYSCALL(TERMINATE, ZERO, ZERO, ZERO);
    }else{
        /* mutual exclusion of the swap pool table */
        SYSCALL(PASSEREN, &swapSem, ZERO, ZERO);
        int missingPgNo = (exceptionState->s_entryHI & VPNMASK);
        int frame = pickVictim();
        /* selected frame is used */
        memaddr page = (memaddr) (SWPSTARTADDR + ((frame)* PAGESIZE));
        /* if the frame is used, it is swapped out */
        if(swapPool[frame].sw_asid!=-ONE){
            /* atomic operation */
            interruptsSwitch(0);
            swapPool[frame].sw_pte->entryLO &= ~(VALIDON);
            TLBCLR();
            interruptsSwitch(1);
            /* write out the used page to flash */
            flashIO(1, frame, page, swapPool[frame].sw_asid-ONE);
        }
        /* read the missing page from flash */
        flashIO(0, missingPgNo, page, support->sup_asid-ONE);
        swapPool[frame].sw_asid = support->sup_asid;
        swapPool[frame].sw_pageNo = missingPgNo;
        swapPool[frame].sw_pte = &(supports[support->sup_asid-ONE].sup_privatPgTb[missingPgNo]);
        interruptsSwitch(0);
        swapPool[frame].sw_pte->entryLO = page | DIRTYON | VALIDON;
        TLBCLR();
        interruptsSwitch(1);
        SYSCALL(VERHOGEN, &swapSem, ZERO, ZERO);
        LDST(BIOSDATAPAGE);  
    }
}
void debug(int a, int b ){
    3+4;
}
void uTLB_RefillHandler(){
    state_PTR exceptionState = BIOSDATAPAGE;
    int asid = ((exceptionState->s_entryHI & GETASID) >> ASIDSHIFT);
    int missingPgNo = supports[asid-ONE].sup_exceptState[PGFAULTEXCEPT].s_entryHI & VPNMASK;
    setENTRYHI(supports[asid-ONE].sup_privatPgTb[missingPgNo].entryHI);
	setENTRYLO(supports[asid-ONE].sup_privatPgTb[missingPgNo].entryLO);
    TLBWR();
    LDST(BIOSDATAPAGE);
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
    int res = SYSCALL(WAITIO, FLASHINT, flashIOdNo, 0);
    if (res!=READY){
        SYSCALL(TERMINATE, ZERO, ZERO, ZERO);
    }
    /*SYSCALL(VERHOGEN, (int)&devicesSem[8+flashIOdNo], 0, 0);*/				/* V(flash_mut) */

}