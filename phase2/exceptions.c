#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"

/**************************************************************************************************************/
/*                                            EXCEPTIONS.c                                                    */
/* This module contains the methods for non IO exception handling in phase 2. SYSCALLs 1-8 are handled in     */
/* this module and all other exceptions such as TLB, program trap, and other SYSCALLs are passed up to be     */
/* handled in phase 3. This module also contains the stateCopy function which is used in other modules.       */
/* The 8 SYSCALLs handled in this module will be defined in detail below.                                     */
/**************************************************************************************************************/

/*Global Variables*/
extern pcb_PTR currentProc;
extern pcb_PTR readyQueue;
extern int processCount;
extern int softBlockCount;
extern int semDevices[DEVNUM];
extern cpu_t startTOD;
extern int * clockSem;
extern void loadState(state_PTR ps);

/*Declaration of SYSCALL Functions*/
void createProc(state_PTR curr);
void terminateProc(pcb_PTR prnt);
void pass(state_PTR curr);
void ver(state_PTR curr);
void waitForIO(state_PTR curr);
void getCPUTime(state_PTR curr);
void waitForClock(state_PTR curr);
void getSupport(state_PTR curr);

/*Declaration of Helper Functions*/
void passUpOrDie(state_PTR curr, int exception);
void otherExceptions();
void pgrmTrap();
void tblTrab();
void stateCopy(state_PTR oldState, state_PTR newState);

/*
Main method to handle SYSCALLS. 8 SYSCALLS total (1-8).
	Parameters: NONE
	RETURN: NONE
*/
void SYSCALLHandler(){
    state_PTR ps = (state_PTR) BIOSDATAPAGE;
    /* add 4 to pc */
    ps->s_t9 = ps->s_pc = ps->s_pc+PCINC;
    /* if UM bits were ON the we passup or die */
    int mode = (ps->s_status & UMOFF);
    if(mode != ALLOFF){
	    passUpOrDie(ps, GENERALEXCEPT);
    }
    
    switch (ps->s_a0)
    {
    case CREATEPROCESS:{
        createProc(ps);
        break;}
    
    case TERMINATEPROCESS:{
        if(currentProc != NULL){
            terminateProc(currentProc);
        }
        scheduler();
        break;}
    
    case PASSEREN:{
        pass(ps);
        break;}
    
    case VERHOGEN:{
        ver(ps);
        break;}
    
    case WAITIO:{
        waitForIO(ps);
        break;}
    
    case GETCPUTIME:{
        getCPUTime(ps);
        break;}
    
    case WAITCLOCK:{
        waitForClock(ps);
        break;}

    case GETSUPPORTPTR:{
        getSupport(ps);
        break;}
    
    default:{
        passUpOrDie(ps, GENERALEXCEPT);
        break;}
    }
}

/* 
SYSCALL 1 Method. creates a new process and sets it as the child of the current proccess.
	Parameter: state_PTR oldState
	Return: -1 if unsucessful
		0 if sucessful
*/
void createProc(state_PTR curr){
    /* allocate a new PCB or process */
    pcb_PTR child = allocPcb();
    /* set return status to -1 to indicate failure as default*/
    int returnStatus = -1;
    /* if allocate was a success */
    if(child != NULL){
        /* make the new process a child of the current process */
        insertChild(currentProc, child);
        /* insert new process into ready queue */
        insertProcQ(&readyQueue, child);
        /* copy the passed state into the state of the new process */
        stateCopy((state_PTR) (curr->s_a1), &(child->p_s));
        /* set the support structure of the new process if it was passed else it is set to NULL*/
        if(curr->s_a2 != 0 || curr->s_a2 != NULL){
            child->p_supportStruct = (support_t *) curr->s_a2;
        } else {
            child->p_supportStruct = NULL;
        }
        /* increment process count */
        processCount++;
        /* set return status to 0 to indicate success*/
        returnStatus = 0;
    }
    /* return returnStatus */
    currentProc->p_s.s_v0 = returnStatus;
    loadState(curr);   
}

/* 
SYSCALL 2 Method. murders the current process and its decendants.
	Parameter: state_PTR oldState
	Return: NULL 
*/
void terminateProc(pcb_PTR curr){
    while(!emptyChild(curr)){
	    terminateProc(removeChild(curr));
    }
     /*check if its the current proccess*/
    if(currentProc == curr){
	    outChild(curr);
    } 
    /*check if in readyQueue*/
    if(curr->p_semAdd == NULL){
	    outProcQ(&readyQueue, curr);
    }
   else {
        /*this bitch hiding in semaphores*/
        int* semdAdd = curr->p_semAdd;
        pcb_PTR r = outBlocked(curr);
        if(r != NULL){
            if( semdAdd >= &semDevices[ZERO] && semdAdd <= &semDevices[DEVNUM]){
                softBlockCount--;
            } else {
                (*semdAdd)++;
            }	
        }
        
    }
    freePcb(curr);
    processCount--;
}

/*
SYSCALL 3 Method. Passeren, used when a process needs mutual exclusion on variable.
if another process already has mutual exclusion on the variable then process is put into
semaphores to wait.
	Parameters: state_PTR oldState
	Return: NULL
*/
void pass(state_PTR curr){
    int* semdAdd = curr->s_a1;
    /* decrement semaphore */
    (*semdAdd)--;
    /* if semaphore less than zero, then block the process */
    if((*semdAdd)<0){
	    stateCopy(curr, &(currentProc->p_s));
        insertBlocked(semdAdd, currentProc);
        scheduler();
    }
    loadState(curr);   
}

/*SYSCALL 4 Method. Verhogen, used when a process is done with mutual exclusion on variable.
 checks if there is another process waiting for mutual exclusion, and if so pulls it out of
 the semaphore and puts it onto the readyQueue
	Parameters: state_PTR oldState
	Return: NULL
*/
void ver(state_PTR curr){
    int* semdAdd = curr->s_a1;
    /* increment semaphore */
    (*semdAdd)++;
    /* semaphore is  less than or equal to zero unblock a procress */
    if((*semdAdd)<=0){
        pcb_PTR temp = removeBlocked(semdAdd);
        if(temp != NULL) {
            insertProcQ(&readyQueue, temp);
        }
    }
    loadState(curr);
}

/*
SYSCALL 5 Method. called when process needs to wait for device I/O.
Process is put onto that devices respective semaphore to wait.
	Parameters: state_PTR oldState
	Return: NULL
*/
void waitForIO(state_PTR curr){
    stateCopy(curr, &(currentProc->p_s));
    /* get line number of device */
    int lineNo = curr->s_a1;
    /* get device number */
    int devNo = curr->s_a2;
    int waitterm = curr->s_a3;
    /* calculate device's semaphore index */
    int devi = ((lineNo - 3 + waitterm) * DEVPERINT + devNo);
    /* block current process */
    semDevices[devi]--;
    softBlockCount++;
    insertBlocked(&(semDevices[devi]), currentProc);
    currentProc = NULL;
    scheduler();
}

/*
SYSCALL 6 Method. Called when process needs the CPU Time.
CPU Time is stored in the processes pcb time and its states v0.
	Parameters: state_PTR oldState
	Return: NULL 
*/
void getCPUTime(state_PTR curr){
    /* copy exception state to current process state */
    stateCopy(curr, &(currentProc->p_s));
    cpu_t time;
    /* calculate time that has passed by subtracting current time from startTOD*/
    STCK(time);
    time -= startTOD;
    /* add time to current process time */
    currentProc->p_time += time;
    currentProc->p_s.s_v0 = currentProc->p_time;
    loadState(&currentProc->p_s);   
}

/*
SYSCALL 7 Method. Called when process needs to wait for the interval
timer. Process is stored on the interval timers Semaphore.
	Parameters: state_PTR oldState
	Return: NULL
*/
void waitForClock(state_PTR curr){
    stateCopy(curr, &(currentProc->p_s));
    (*clockSem)--;
    /* if semaphore is less than zero, then block the process in the clock semaphore */
    if((*clockSem)<0){
        insertBlocked(clockSem, currentProc);
        softBlockCount++;
        currentProc = NULL;
        scheduler();
    }
}

/*
SYSCALL 8 Method. Called when the process needs its support structure.
Support structure is stored in the processes pcb supportStruct.
	Parameters: state_PTR oldState
	Return: NULL
*/
void getSupport(state_PTR curr){
    /* copy exception state to current process state */
    stateCopy(curr, &(currentProc->p_s));
    /* return the support structure of the current process */
    currentProc->p_s.s_v0 = currentProc->p_supportStruct;
    loadState(&currentProc->p_s);   
}

/*
Method called when exception cannot be handled in this phase.
If the current processes support struct is NULL the process and
its children are all murdered. otherwisep asses up the context 
and what type of exception to be handled in phase 3.
	Parameters: state_PTR oldState
		    int exception -> type of exception is being passed up
	Return: NULL
*/
void passUpOrDie(state_PTR curr, int exception){
    /* if the current process has no support structure then we die */
    if(currentProc->p_supportStruct == NULL){
        terminateProc(currentProc); 
        currentProc = NULL;
        scheduler();
    }else{
        /*i think we will need a switch to figure out which except state to put the biosdatapage in */
        stateCopy(curr, &(currentProc->p_supportStruct->sup_exceptState[exception]));
        LDCXT(currentProc->p_supportStruct->sup_exceptContext[exception].c_stackPtr, currentProc->p_supportStruct->sup_exceptContext[exception].c_status, currentProc->p_supportStruct->sup_exceptContext[exception].c_pc);   
    }
}

/*
Copies the state of one state into another.
	Parameters: state_PTR oldState
		    state_PTR newState
	Return: NULL
*/
void stateCopy(state_PTR oldState, state_PTR newState){
    int i;
    /* copy the registers */
    for(i=0; i<STATEREGNUM; i++){
	    newState->s_reg[i] = oldState->s_reg[i];
    }
    /* copy the other special registersa */
    newState->s_entryHI = oldState->s_entryHI;
    newState->s_cause = oldState->s_cause;
    newState->s_status = oldState->s_status;
    newState->s_pc = oldState->s_pc;
}

/*
Helper method to handle non syscall 1-8 exceptions
	Parameters: int reason -> what type of exception is it
	Return: NULL
*/
void otherExceptions(int reason){
    if(reason<4){
        passUpOrDie((state_PTR) BIOSDATAPAGE,  PGFAULTEXCEPT);
    } else{
        passUpOrDie((state_PTR) BIOSDATAPAGE,  GENERALEXCEPT);
    }
}
