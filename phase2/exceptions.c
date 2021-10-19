#include "../h/types.h"
#include "../h/const.h"
#include "../h/pcb.h"
#include "../h/asl.h"
#include "../h/scheduler.h"
#include "../h/exceptions.h"

extern currentProc;
extern readyQueue;
extern processCount;

HIDDEN int createProc();
HIDDEN void terminateProc(pcb_PTR prnt);
HIDDEN void pass();
HIDDEN void ver();
HIDDEN void waitForIO();
HIDDEN void getCPUTime();
HIDDEN void waitForClock();
HIDDEN void getSupport();

void SYSCALL(){
    
    switch (a0)
    {
    case CREATEPROCESS:
        returnExceptionState(createProc);
        break;
    
    case TERMINATEPROCESS:
        terminateProc(currentProc);
        processCount--;
        scheduler();
        break;
    
    case PASSEREN:
        /* code */
        break;
    
    case VERHOGEN:
        /* code */
        break;
    
    case WAITIO:
        /* code */
        break;
    
    case GETCPUTIME:
        /* code */
        break;
    
    case WAITCLOCK:
        /* code */
        break;

    case GETSUPPORTPTR:
        /* code */
        break;
    
    default:
        break;
    }
}
/* Utility function takes in parent process*/
int createProc(){
pcb_PTR child = allocPcb();
        int returnStatus = -1;
        if(child != NULL){
            insertChild(currentProc, child);
            insertProcQ(readyQueue, child);
            child->p_s = *((state_PTR) a1);
            if(a2 != 0){
                child->p_supportStruct = (support_t *) a2;
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

void pass(){
/*
mutex--
if(mutex<0){
insertBlocked(&mutex,currentProc()){
scheduler
*/
}

void ver(){
/*
mutex++
if(mutex<=0){
temp = removeBlicked(&mutex)
insertProcQ(&readyQ,temp)
*/
}

void waitForIO(){

}

void getCPUTime(){

}

void waitForClock(){

}

HIDDEN void getSupport(){

}

void returnExceptionState(int returnVal){
        state_PTR except_state;
        except_state = (state_PTR) 0x0FFF0000;
        except_state->s_v0 = returnVal;
        /* add 4 to pc */
        except_state->s_t9 = except_state->s_pc+=4;
}
