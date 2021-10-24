#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"
#include "../h/initial.h"

extern currentProc;
extern readyQueue;
extern processCount;
extern softBlockCount;
extern semDevices;
extern loadState;
extern startTOD;
extern clockSem;

HIDDEN int createProc(state_PTR curr);
HIDDEN void terminateProc(pcb_PTR prnt);
HIDDEN void pass(state_PTR curr);
HIDDEN void ver(state_PTR curr);
HIDDEN void waitForIO(state_PTR curr);
HIDDEN void getCPUTime(state_PTR curr);
HIDDEN void waitForClock(state_PTR curr);
HIDDEN void getSupport(state_PTR curr);
HIDDEN void passUpOrDie(state_PTR curr);
void stateCopy(state_PTR oldState, state_PTR newState);
void pgrmTrap();
void tblTrab();

void SYSCALLHandler(){
    state_PTR ps = (state_PTR) BIOSDATAPAGE;
    switch (ps->s_a0)
    {
    case CREATEPROCESS:{
        returnExceptionState(createProc(ps));
        break;}
    
    case TERMINATEPROCESS:{
        terminateProc(currentProc);
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
        passUpOrDie(ps);
        break;}
    }
}
/* Utility function takes in parent process*/
int createProc(state_PTR curr){
    pcb_PTR child = allocPcb();
    int returnStatus = -1;
    if(child != NULL){
        insertChild(currentProc, child);
        insertProcQ(readyQueue, child);
        copyState(curr->s_s1, &(child->p_s));
        if(curr->s_a2 != 0 || curr->s_a2 != NULL){
            child->p_supportStruct = (support_t *) curr->s_a2;
        } else {
            child->p_supportStruct = NULL;
        }
        child->p_time = 0;
        child->p_semAdd = NULL;
        processCount++;
        returnStatus = 0;
    }
    return returnStatus;
}

void terminateProc(pcb_PTR curr){
    while(!emptyChild(curr)){
	terminateProc(removeChild(curr));
    }
/*check if in readyQueue*/
    if(curr->p_semAdd == NULL){
	outProcQ(&readyQueue, curr);
    }
/*check if its the current proccess*/
    if(currentProc == curr){
	outChild(curr);
    }
    /*this bitch hiding in semaphores*/
        else{
        int* semdAdd = curr->p_semAdd;
        outBlocked(curr);
        if( semdAdd >= semDevices[ZERO] && semdAdd <= semDevices[DEVNUM]){
            softBlockCount--;
        }
        else{
            semdAdd++;	
        }
        
    }
    freePcb(curr);
    processCount --;
}

void pass(state_PTR curr){
    int* semdAdd = curr->s_a1;
    *semdAdd --;
    if(semdAdd<0){
	copyState(ps, &(currentProcess->p_s));
        insertBlocked(semdAdd, currentProc);
        scheduler();
    }
    incrementPC();
}

void ver(state_PTR curr){
    int* semdAdd = curr->s_a1;
    *semdAdd ++;

    if(semdAdd<=0){
        pcb_PTR temp = removeBlocked(semdAdd);
	if(temp != NULL) {
        insertProcQ(&readyQueue, temp);
	}
    }
    incrementPC();
}
void waitForIO(state_PTR curr){
    copyState(ps, &(currentProcess->p_s));
    int lineNo = curr->s_a1;
    int devNo = curr->s_a2;
    int waitterm = curr->s_a3;
    int devi = ((lineNo) * (devNo-TWO));
    semDevices[devi]++;
    softBlockCount++;
    insertBlocked(&devi, currentProc);
    scheduler();
}

void getCPUTime(state_PTR curr){
    copyState(ps, &(currentProcess->p_s));
    int time;
    STCK(time);
    time -= startTOD;
    currentProc->p_time = time;
    incrementPC();
}

void waitForClock(state_PTR curr){
    copyState(ps, &(currentProcess->p_s));
    clockSem --;
    insertBlocked(clockSem, currentProc);
    softBlockCount++;
    scheduler();
}

void getSupport(state_PTR curr){
    returnExceptionState(currentProc->p_supportStruct);
}

void returnExceptionState(int returnVal){
        state_PTR except_state;
        except_state = (state_PTR) BIOSDATAPAGE;
        except_state->s_v0 = returnVal;
        /* add 4 to pc */
        incrementPC();
}
void incrementPC(){
    state_PTR except_state = (state_PTR) BIOSDATAPAGE;
    except_state->s_t9 = except_state->s_pc+=PCINC;
    loadState(currentProc->p_s);   
}

void passUpOrDie(state_PTR curr){
    if(currentProc->p_supportStruct == NULL){
        terminateProc(currentProc); 
    } 
    currentProc->p_supportStruct.sup_exceptState = curr;
    LDCXT(currentProc->p_supportStruct.sup_exceptContext);   
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

