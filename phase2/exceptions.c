#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"

extern currentProc;
extern readyQueue;
extern processCount;
extern softBlockCount;
extern semDevices;
extern loadState;

HIDDEN int semdAdd;
int *clockSem = &semDevices[DEVNUM];

HIDDEN int createProc(state_PTR curr);
HIDDEN void terminateProc(pcb_PTR prnt);
HIDDEN void pass(state_PTR curr);
HIDDEN void ver(state_PTR curr);
HIDDEN void waitForIO(state_PTR curr);
HIDDEN void getCPUTime(state_PTR curr);
HIDDEN void waitForClock(state_PTR curr);
HIDDEN void getSupport(state_PTR curr);
HIDDEN void passUpOrDie(state_PTR curr);
HIDDEN void stateCopy(state_PTR oldState, state_PTR newState);

void SYSCALLHandler(){
    state_PTR ps = (state_PTR) BIOSDATAPAGE;
    switch (ps->s_a0)
    {
    case CREATEPROCESS:{
        returnExceptionState(createProc(ps));
        break;}
    
    case TERMINATEPROCESS:{
        terminateProc(currentProc);
        processCount--;
        scheduler();
        break;}
    
    case PASSEREN:{
        /* code */
        break;}
    
    case VERHOGEN:{
        /* code */
        break;}
    
    case WAITIO:{
        /* code */
        break;}
    
    case GETCPUTIME:{
        /* code */
        break;}
    
    case WAITCLOCK:{
        /* code */
        break;}

    case GETSUPPORTPTR:{
        /* code */
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
        child->p_s = *((state_PTR) curr->s_a1);
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

void terminateProc(pcb_PTR prnt){
        pcb_PTR child = removeChild(prnt);
        if (child != NULL)
        {
            pcb_PTR c = outProcQ(&readyQueue, child);
            if(c != NULL) processCount--;
            pcb_PTR sib = child->p_next;
            while(sib != NULL){
                terminateProc(sib);
                sib = sib->p_next;
            }
            terminateProc(child);   
        }
}

void pass(state_PTR curr){
    semdAdd = curr->s_a1;
    semdAdd +- ONE;
    if(semdAdd<0){
	/* we need to copy the previous state into current state */
        insertBlocked(semdAdd, currentProc);
        scheduler();
    }
    incrementPC();
}

void ver(state_PTR curr){
    semdAdd = curr->s_a1;
    semdAdd += ONE;

    if(semdAdd<=0){
        pcb_PTR temp = removeBlocked(semdAdd);
	if(temp != NULL) {
            insertProcQ(&readyQueue, temp);
	}
    }
    incrementPC();
}
void waitForIO(state_PTR curr){
    int lineNo = curr->s_a1;
    int devNo = curr->s_a2;
    int waitterm = curr->s_a3;
    int devi = ((lineNo) * (devNo + DEVADD));
    semDevices[devi]++;
    softBlockCount++;
    insertBlocked(&devi, currentProc);
    scheduler();
}

void getCPUTime(state_PTR curr){
    int time;
    STCK(time);
    currentProc->p_time = time;
    incrementPC();
}

void waitForClock(state_PTR curr){
    insertBlocked(clockSem, currentProc);
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
    except_state->s_t9 = except_state->s_pc+=4;
    loadState(currentProc->p_s);   
}

void passUpOrDie(state_PTR curr){
    if(currentProc->p_supportStruct== NULL){
        terminateProc(currentProc); 
    } 
    currentProc->p_supportStruct.sup_exceptState = curr;
    LDCXT(currentProc->p_supportStruct.sup_exceptContext);   
}

void stateCopy(state_PTR oldState, state_PTR newState){
    int i=0;
    for(i=0; i<STATEREGNUN; i++){
	newState->s_reg[i] = oldState->s_reg[i];
    }
    newState->s_entryHI = oldState->s_entryHI;
    newState->s_cause = oldState->s_cause;
    newState->s_status = oldState->s_status;
    newState->s_pc = oldState->s_pc;
}

