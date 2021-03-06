#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"

extern pcb_PTR currentProc;
extern pcb_PTR readyQueue;
extern int processCount;
extern int softBlockCount;
extern int semDevices[DEVNUM];
extern cpu_t startTOD;
extern int * clockSem;
extern void loadState(state_PTR ps);

void createProc(state_PTR curr);
void terminateProc(pcb_PTR prnt);
void pass(state_PTR curr);
void ver(state_PTR curr);
void waitForIO(state_PTR curr);
void getCPUTime(state_PTR curr);
void waitForClock(state_PTR curr);
void getSupport(state_PTR curr);
void passUpOrDie(state_PTR curr, int exception);

void otherExceptions();
void pgrmTrap();
void tblTrab();
void stateCopy(state_PTR oldState, state_PTR newState);

void SYSCALLHandler(){
    state_PTR ps = (state_PTR) BIOSDATAPAGE;
    /* add 4 to pc */
    ps->s_t9 = ps->s_pc = ps->s_pc+PCINC;

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
/* Utility function takes in parent process*/
void createProc(state_PTR curr){
    pcb_PTR child = allocPcb();
    int returnStatus = -1;
    if(child != NULL){
        insertChild(currentProc, child);
        insertProcQ(&readyQueue, child);
        stateCopy((state_PTR) (curr->s_a1), &(child->p_s));
        if(curr->s_a2 != 0 || curr->s_a2 != NULL){
            child->p_supportStruct = (support_t *) curr->s_a2;
        } else {
            child->p_supportStruct = NULL;
        }
        processCount++;
        returnStatus = 0;
    }
    currentProc->p_s.s_v0 = returnStatus;
    loadState(curr);   
}

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

void pass(state_PTR curr){
    int* semdAdd = curr->s_a1;
    (*semdAdd)--;
    if((*semdAdd)<0){
	    stateCopy(curr, &(currentProc->p_s));
        insertBlocked(semdAdd, currentProc);
        scheduler();
    }
    loadState(curr);   
}

void ver(state_PTR curr){
    int* semdAdd = curr->s_a1;
    (*semdAdd)++;
    if((*semdAdd)<=0){
        pcb_PTR temp = removeBlocked(semdAdd);
        if(temp != NULL) {
            insertProcQ(&readyQueue, temp);
        }
    }
    loadState(curr);
}
void waitForIO(state_PTR curr){
    stateCopy(curr, &(currentProc->p_s));
    int lineNo = curr->s_a1;
    int devNo = curr->s_a2;
    int waitterm = curr->s_a3;
    int devi = ((lineNo - 3 + waitterm) * DEVPERINT + devNo);
    semDevices[devi]--;
    softBlockCount++;
    insertBlocked(&(semDevices[devi]), currentProc);
    currentProc = NULL;
    scheduler();
}

void getCPUTime(state_PTR curr){
    stateCopy(curr, &(currentProc->p_s));
    cpu_t time;
    STCK(time);
    time -= startTOD;
    currentProc->p_time += time;
    currentProc->p_s.s_v0 = currentProc->p_time;
    loadState(&currentProc->p_s);   
}

void waitForClock(state_PTR curr){
    stateCopy(curr, &(currentProc->p_s));
    (*clockSem)--;
    if((*clockSem)<0){
        insertBlocked(clockSem, currentProc);
        softBlockCount++;
        currentProc = NULL;
        scheduler();
    }
}

void getSupport(state_PTR curr){
    stateCopy(curr, &(currentProc->p_s));
    currentProc->p_s.s_v0 = currentProc->p_supportStruct;
    loadState(&currentProc->p_s);   
}


void passUpOrDie(state_PTR curr, int exception){
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

void stateCopy(state_PTR oldState, state_PTR newState){
    int i=0;
    for(i=0; i<STATEREGNUM; i++){
	    newState->s_reg[i] = oldState->s_reg[i];
    }
    newState->s_entryHI = oldState->s_entryHI;
    newState->s_cause = oldState->s_cause;
    newState->s_status = oldState->s_status;
    newState->s_pc = oldState->s_pc;
}

void otherExceptions(int reason){
    if(reason<4){
        passUpOrDie((state_PTR) BIOSDATAPAGE,  PGFAULTEXCEPT);
    } else{
        passUpOrDie((state_PTR) BIOSDATAPAGE,  GENERALEXCEPT);
    }
}
