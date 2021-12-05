#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"
#include "../h/interrupts.h"

extern semDevices[DEVNUM];
extern int swapSem;
extern swap_t swapPool [POOLSIZE];
extern void stateCopy(state_PTR oldState, state_PTR newState);

void uSyscallHandler(){
    support_t * support = SYSCALL(GETSUPPORTPTR, ZERO, ZERO, ZERO);
    state_t exceptionState;
    stateCopy(&support->sup_exceptState[GENERALEXCEPT], &exceptionState);
    exceptionState.s_pc+=PCINC;
    int cause = exceptionState.s_cause;
    if(cause == 1){
        SYSCALL(TERMINATE, ZERO, ZERO, ZERO);
    }else{
        int syscallNum = exceptionState.s_a0;
        int pid = support->sup_asid;
        int arg1 = exceptionState.s_a1;
        int arg2 = exceptionState.s_a2;
        int arg3 = exceptionState.s_a3;
        int ret = 0;
        switch(syscallNum){
            case TERMINATE:
                terminate(pid);
                break;
            case GetTOD:
                exceptionState.s_v0 = getTOD();
                break;
            case WRITETOPRINTER:
                exceptionState.s_v0 = writeToPrinter(arg1, arg2, pid);
                break;
            case WRITETOTERMINAL:
                exceptionState.s_v0 = writeToTerminal(arg1, arg2, pid);
                break;
            case READFROMTERMINAL:
                exceptionState.s_v0 =  readFromTerminal(arg1);
                break;
            default:
                terminate(pid);
        }
        LDST(&exceptionState);
    }
}



void interruptsSwitch(int on){
    if(on){
        setSTATUS(getSTATUS() | IEON);
    } else {
        setSTATUS(getSTATUS() & ~IEON);
    }
}

terminate(int asid){
    SYSCALL(VERHOGEN,&swapSem,ZERO,ZERO);
    interruptsSwitch(0);
        int i; 
        for(i = 0; i < POOLSIZE;i++){
            if(swapPool[i].sw_asid == asid ){
                swapPool[i].sw_asid = -ONE;
                swapPool[i].sw_pageNo = -ONE;
                swapPool[i].sw_pte = NULL;
            }

        }
    interruptsSwitch(1);
    
    SYSCALL(PASSEREN,&swapSem,ZERO,ZERO);
    
    TLBCLR(); 
    SYSCALL(TERMINATEPROCESS,ZERO,ZERO,ZERO);
}

int writeToTerminal(char *msg, int strlen, int pid) {
	unsigned int * base = (unsigned int *) (TERM0ADDR);
	unsigned int status;
	SYSCALL(PASSEREN, (int)&semDevices[34], 0, 0);				/* P(term_mut) */
    int i = 0;
    int ret = 0;
	for (i;i<strlen;i++) {
		*(base + 3) = PRINTCHR | (((unsigned int) *msg) << BYTELEN);
		int status = SYSCALL(WAITIO, TERMINT, 0, 0);	
        if ((status & TERMSTATMASK) != RECVD){
			ret = -status;
            break;
        }
		msg++;	
	}
	SYSCALL(VERHOGEN, (int)&semDevices[34], 0, 0);				/* V(term_mut) */
    return ret;
}
int readFromTerminal(char * virtAddr){
    return 0;
}


int writeToPrinter(char *msg, int strlen, int pid) {
    unsigned int * printer = (0x10000054 + ((3) * 0x80) + (pid * 0x10));
    unsigned int status;
	SYSCALL(PASSEREN, (int)&semDevices[34], 0, 0);				/* P(term_mut) */
    int i = 0;
    int ret = 0;
	for (i;i<strlen;i++) {
		*(printer + 3) = PRINTCHR | (((unsigned int) *msg) << BYTELEN);
		int status = SYSCALL(WAITIO, TERMINT, 0, 0);	
        if ((status & TERMSTATMASK) != RECVD){
			ret = -status;
            break;
        }
		msg++;	
	}
	SYSCALL(VERHOGEN, (int)&semDevices[34], 0, 0);				/* V(term_mut) */
    return ret;
    return 0;
}

cpu_t getTOD(){
    cpu_t now;
    STCK(now);
    return now;
}
