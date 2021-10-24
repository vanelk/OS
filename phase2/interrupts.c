#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"

extern int semDevices[DEVNUM];
extern pcb_PTR readyQueue;
extern pcb_PTR currentProc;
extern int softBlockCount;
extern int * clockSem;
extern cpu_t startTOD;
extern void stateCopy(state_PTR oldState, state_PTR newState);
cpu_t stopTOD;

void IOHandler(){
    state_PTR  exception_state = (state_PTR) BIOSDATAPAGE;
    int ip_bits = (exception_state->s_cause >> 8) << 16;
    int intlNo = 0;
    if(ip_bits & 1){
       PANIC();
    } else if (ip_bits & 2) {
        prepToSwitch();
    } else if (ip_bits & 4) {
        /* ACK the interrupt */
        LDIT(QUANTUM);
        pcb_PTR proc = removeBlocked(clockSem);
        /* Unblock all processes on the pseudo-clock */
        while (proc!=NULL)
        {
            insertProcQ(&readyQueue, proc);
            pcb_PTR proc = removeBlocked(clockSem);
        }
        /* reset clock semaphore */
        *clockSem = 0;
        prepToSwitch();
    } else if (ip_bits & 8) { 
        intlNo = 3;
    } else if (ip_bits & 16) {
        intlNo = 4;
    } else if (ip_bits & 32) {
        intlNo = 5;
    } else if (ip_bits & 64) {
        intlNo = 6;
    } else if (ip_bits & 128) {
        intlNo = 7;
    }
    devregarea_t * ram = (devregarea_t *) RAMBASEADDR;
    int dev_bits = ram->interrupt_dev[intlNo-3];
    int devNo;
    if(dev_bits & 1){
        devNo = 0;
    } else if (dev_bits & 2) {
        devNo = 1;
    } else if (dev_bits & 4) {
        devNo = 2;
    } else if (dev_bits & 8) {
        devNo = 3;
    } else if (dev_bits & 16) {
        devNo = 4;
    } else if (dev_bits & 32) {
        devNo = 5;
    } else if (dev_bits & 64) {
        devNo = 6;
    } else if (dev_bits & 128) {
        devNo = 7;
    }
    /* Non timer interrupts */
    if( intlNo >= 3){
        int devIdx = (intlNo - 3) * 10 + devNo;
        /* calculate the device address */
        int devAddrbase = 0x10000054 + ((intlNo -3) * 0x80) + (devNo * 0x10);
        /* Get the device from the device address */
        int statusCp;
        device_t * dev = (device_t *) devAddrbase;
        /* terminal device */
        if (intlNo == 7){
            /* ready state for transmit*/
            if(dev->t_transm_status & 16){
                /* ACK the command */
                dev->t_transm_command = ACK;
                statusCp = dev->t_transm_status;
            } else {
                /* ACK the recieve command */
                dev->t_recv_command = ACK;
                statusCp = dev->t_recv_status;
            }
        } else {
            /* Store the status code from the device register */
            statusCp = dev->d_status;
            /* ACK the interupt */
            dev->d_command = ACK;
        }
        /* Do a V on the dev sem for current device */
        int * semad = &semDevices[devIdx];
        (*semad)+=ONE;
        if(semad<=ZERO){
            pcb_PTR proc = removeBlocked(semad);
            if(proc!=NULL){
                proc->p_s.s_v0 = statusCp;
                softBlockCount-=ONE;
                insertProcQ(&readyQueue, proc);
            }
        }
        /* probably do a reschedule? since we need to think about the time*/
        prepToSwitch();
    }

}

void prepToSwitch(){
    state_PTR  exception_state = (state_PTR) BIOSDATAPAGE;
    if(currentProc!=NULL){
        stateCopy(&currentProc->p_s, exception_state);
        insertProcQ(&readyQueue, currentProc);
    }
    scheduler();
}