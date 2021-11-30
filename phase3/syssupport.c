#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"
#include "../h/interrupts.h"

extern semDevices[DEVNUM];

void uSyscallHandler(){
    support_t * support = SYSCALL(GETSUPPORTPTR, ZERO, ZERO, ZERO);
    state_t exceptionState = support->sup_exceptState[GENERALEXCEPT];
    exceptionState.s_pc+=PCINC;
    int cause = exceptionState.s_cause;
    if(cause == 1){
        SYSCALL(TERMINATEPROCESS, ZERO, ZERO, ZERO);
    }else{
        int syscallNum = exceptionState.s_a0;
        int pid = support->sup_asid;
        int arg1 = exceptionState.s_a1;
        int arg2 = exceptionState.s_a2;
        int arg3 = exceptionState.s_a3;
        int ret = 0;
        switch(syscallNum){
            case TERMINATE:
                terminate();
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
                terminate();
        }
        LDST(&exceptionState);
    }
}


int pickVictim(){
    static int i=0;
    i=(i+1)%POOLSIZE;
    return i;
}

void interruptsSwitch(int on){
    if(on){
        setSTATUS(getSTATUS() | IEON);
    } else {
        setSTATUS(getSTATUS() & ~IEON);
    }
}

terminate(){
    
    SYSCALL(TERMINATEPROCESS, ZERO, ZERO, ZERO);
}

int writeToTerminal(char *msg, int strlen, int pid) {

	char *s = msg;
	unsigned int * base = (unsigned int *) (TERM0ADDR);
	unsigned int status;
	SYSCALL(PASSEREN, (int)&semDevices[34], 0, 0);				/* P(term_mut) */
    int i = 0;
    int ret = 0;
	for (i;i<strlen;i++) {
		*(base + 3) = PRINTCHR | (((unsigned int) *s) << BYTELEN);
		int status = SYSCALL(WAITIO, TERMINT, 0, 0);	
        if ((status & TERMSTATMASK) != RECVD){
			ret = -status;
            break;
        }
		s++;	
	}
	SYSCALL(VERHOGEN, (int)&semDevices[34], 0, 0);				/* V(term_mut) */
    return ret;
}
int readFromTerminal(char * virtAddr){
    return 0;
}


int writeToPrinter(char *msg, int strlen, int pid) {
    device_t* printer = (device_t *) (0x10000054 + ((3) * 0x80) + (pid * 0x10));
    return 0;
}

cpu_t getTOD(){
    cpu_t now;
    STCK(now);
    return now;
}

void flashIO(int writeOn, int data, int flashIOdNo){
    device_t* flash = (device_t *) ((0x10000054 + 0x80) + (flashIOdNo * 0x10));
    SYSCALL(PASSEREN, (int)&semDevices[8+flashIOdNo], 0, 0);				/* P(flash_mut) */
    if(writeOn){
        flash->d_command = 3;
        flash->d_data0 = data;
    }else{
        flash->d_command = 2;
        flash->d_data0 = data;
    }
    SYSCALL(WAITIO, FLASHINT, 0, 0);
    SYSCALL(VERHOGEN, (int)&semDevices[8+flashIOdNo], 0, 0);				/* V(flash_mut) */

}