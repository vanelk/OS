#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"

/**************************************************************************************************************/
/*                                            INTERRUPTS.c                                                    */
/* This module handles IO interrupts. There are 5 different device interrupts along with the interval clock   */
/* interrupt. the interrupt priority begins with the interval timer, then is in order of the devices 3-8      */
/* We decided to not give time back to the interrupted process.                                               */
/**************************************************************************************************************/

/*Global Variables*/
extern int semDevices[DEVNUM];
extern pcb_PTR readyQueue;
extern pcb_PTR currentProc;
extern int softBlockCount;
extern int * clockSem;
extern cpu_t startTOD;

/*Helper Methods*/
extern void stateCopy(state_PTR oldState, state_PTR newState);

/*local stop time of the day*/
cpu_t stopTOD;

/*
Main method to handle IO interrupts. We have 5 different types of
devices with 8 devices each. which device caused the interrupt
can be found within the BIOSDATAPAGE Handles devices 1-4 the same
but handles terminal differently. Processes waiting for device
IO can be found in device semaphores semDevices.
	Parameters: NULL
	Return: NULL
*/
void IOHandler(){
    state_PTR  exception_state = (state_PTR) BIOSDATAPAGE;
    /* get the ip bits from cause register of the execption state*/
    int ip_bits = ((exception_state->s_cause & IPMASK)>> 8);
    int intlNo = 0;
    if(ip_bits & 1){
        /* interprocessor interrupt not handled so we panic */
       PANIC();
    } else if (ip_bits & 2) {
        /* PLT interrupt then we switch to the next process */
        prepToSwitch();
    } else if (ip_bits & 4) {
        /*interval timer interrupt*/
        /* ACK the interrupt */
        LDIT(IOCLOCK);
        /* get the stop time of the day */
        STCK(stopTOD);
        /* Unblock all processes on the pseudo-clock semaphore */
        pcb_PTR proc = removeBlocked(clockSem);
        while (proc!=NULL)
        {
            /* charge the process for the time it was blocked */
            proc->p_time += (stopTOD- startTOD);
            insertProcQ(&readyQueue, proc);
            proc = removeBlocked(clockSem);
            /* decrement the softblock count */
            softBlockCount--;
        }
        /* reset clock semaphore */
        *clockSem = 0;
        /* swtich to next process */
        prepToSwitch();
    }
    /* get the line number */    
    if (ip_bits & 8) { 
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
    /* find the device number */
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
        /* calculate the device index using the line and device number */
        int devi = (intlNo - 3) * DEVPERINT + devNo;
        /* calculate the device address; directly taken from POPS pg28 */
        int devAddrbase = 0x10000054 + ((intlNo -3) * 0x80) + (devNo * 0x10);
        int statusCp;
        /* Get the device from the device address */
        device_t * dev = (device_t *) devAddrbase;
        /* terminal device */
        if (intlNo == 7){
            /* ready state for transmit*/
            if(dev->t_transm_command & 15){
                /* ACK the command */
                statusCp = dev->t_transm_status;
                dev->t_transm_command = ACK;
            } else {
                statusCp = dev->t_recv_status;
                /* ACK the recieve command */
                dev->t_recv_command = ACK;
                devi+=DEVPERINT;
            }
        } else {
            /* Store the status code from the device register */
            statusCp = dev->d_status;
            /* ACK the interupt */
            dev->d_command = ACK;
        }
        /* Do a V on the device sem for current device */
        int *semad = &semDevices[devi];
        (*semad)++;
        if(*semad>=ZERO){
            pcb_PTR proc = removeBlocked(semad);
            if(proc!=NULL){
                /* charge the unblocked process for the time spent */
                STCK(stopTOD);
                proc->p_time += (stopTOD- startTOD);
                /* return the device status*/
                proc->p_s.s_v0 = statusCp;
                /* decrement the soft block count */
                softBlockCount--;
                insertProcQ(&readyQueue, proc);
            }
        }
        /* swtich to next process */
        prepToSwitch();
    }

}

/*
Helper method to switch processes after IO is handled. 
copies necessary state data and puts the IO device onto
readyQueue. then calls scheduler.
	Parameters: NULL
	Return: NULL
*/
void prepToSwitch(){
    state_PTR  exception_state = (state_PTR) BIOSDATAPAGE;
    /* change the current process from running to ready if it is not NULL */
    if(currentProc!=NULL){
        stateCopy(exception_state, &(currentProc->p_s));
        insertProcQ(&readyQueue, currentProc);
    }
    /* switch to next process */
    scheduler();
}
