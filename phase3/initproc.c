#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"
#include "../h/interrupts.h"
#include "../h/initproc.h"

swap_t swapPool [POOLSIZE];
int devicesSem[DEVNUM];
int swapSem = 1;
support_t supports [MAXUPROC+1];
extern pcb_PTR currentProc;
extern void uSyscallHandler();
extern void pager();


void test() {
    int pid = 1;
    state_t procState;
    for (pid; pid<MAXUPROC+ONE; pid++) {
        procState.s_sp = 0xC0000000;
        procState.s_pc = procState.s_t9 = (memaddr)0x800000B0;
        procState.s_status = ALLOFF | KUON | TEBITON | IEON | IMON;
        procState.s_entryHI = pid << ASIDSHIFT;
        int i = 0;
        for(i; i < POOLSIZE; i++) {
           supports[pid].sup_privatPgTb[i].entryHI = ((0x80000 + i) << VPNSHIFT) | (pid << ASIDSHIFT);
           supports[pid].sup_privatPgTb[i].entryLO = ALLOFF | DIRTYON;
        }
        supports[pid].sup_asid = pid;
        supports[pid].sup_privatPgTb[POOLSIZE-1].entryHI = (0xBFFFF << VPNSHIFT) | (pid << ASIDSHIFT);
        supports[pid].sup_exceptContext[GENERALEXCEPT].c_stackPtr = (&supports[pid].sup_stackGen[500]);
        supports[pid].sup_exceptContext[GENERALEXCEPT].c_status = ALLOFF | IEON | TEBITON | IMON;
        supports[pid].sup_exceptContext[GENERALEXCEPT].c_pc =  (memaddr) uSyscallHandler; 
        supports[pid].sup_exceptContext[PGFAULTEXCEPT].c_stackPtr = (&supports[pid].sup_stackTLB[500]);
        supports[pid].sup_exceptContext[PGFAULTEXCEPT].c_status = ALLOFF | IEON | TEBITON | IMON;
        supports[pid].sup_exceptContext[PGFAULTEXCEPT].c_pc =  (memaddr) pager;
        SYSCALL(CREATEPROCESS, (int)&procState, (int) &supports[pid], 0);
    }
    while (TRUE) TRUE;
}
