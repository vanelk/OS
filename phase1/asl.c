#include <limits.h>
#include "../h/pcb.h"
#include "../h/asl.h"
HIDDEN semd_t *semd_h, *semFree_h;


semd_t *allocSem(){
    if(semFree_h == NULL) return NULL;
    semd_t *freed = (semd_t*) semFree_h;
    semFree_h = semFree_h->s_next;
    return freed;
}

/*
*
*
*/
semd_t *deallocSem(semd_t *sem){
    if(semFree_h == NULL){
        semFree_h = sem;
    } else{
        sem->s_next = (semd_t*) semFree_h;
        semFree_h = sem;
    }
    return sem;
}

semd_t *search(int *semAdd){
    semd_t *current = (semd_t*) semd_h;
    while(semAdd > (current->s_next->s_semAdd)){
	    current = current->s_next;
    }

    return current;
}

/*
*
*
*/
void initASL(){
    static semd_t semdTable[MAXPROC+2];
    int i;
    for(i=0; i<MAXPROC; i++){
        deallocSem(&semdTable[i+1]);
    }
    semd_h = (&semdTable[0]);
    semd_h->s_semAdd = (int*) INT_MAX;
    semd_h->s_next = (&semdTable[MAXPROC+1]);
    semd_h->s_next->s_semAdd = (int*) INT_MAX;
}

/*
*
*
*/
int insertBlocked(int *semAdd, pcb_PTR p)
{
    semd_t *daddy = search(semAdd);
    if(daddy->s_next->s_semAdd == semAdd){
        insertProcQ(&(daddy->s_next->s_procQ), p);
    }else{
        semd_t *sem = allocSem();
        if(sem == NULL) return 1;
        sem->s_semAdd = semAdd;
        sem->s_procQ = mkEmptyProcQ();
        insertProcQ(&(sem->s_procQ), p);
        sem->s_next = daddy->s_next;
        daddy->s_next = sem;
    }
    return 0;
}

/*
*
*
*/
pcb_PTR removeBlocked(int *semdAdd){
    semd_t *daddy = search(semdAdd);
        if(daddy->s_next->s_semAdd == semdAdd){
            return removeProcQ(&(daddy->s_next->s_procQ));
            if(emptyProcQ(daddy->s_next->s_procQ)){
                if(daddy->s_next->s_semAdd != INT_MAX){
                    semd_t * removed = daddy->s_next;
                    daddy->s_next = removed->s_next;
                    deallocSem(removed);
                }
            }
        } 
        return NULL;
}

/*
*
*
*/
pcb_PTR outBlocked(pcb_PTR p){
    semd_t *parent = search(p->p_semAdd);

    if(parent->s_semAdd == INT_MAX){
	return NULL;
    }

    if(parent->s_next->s_semAdd == p->p_semAdd){
        pcb_PTR pcbToReturn = outProcQ(&(parent->s_next->s_procQ), p);

	if(emptyProcQ(parent->s_next->s_procQ)){
	    deallocSem(parent->s_next);
	}
	
	    return pcbToReturn;
    }
    else{
        return NULL;
    }
}

/*
*
*
*/
pcb_PTR headBlocked(int *semAdd){
    semd_t *temp = search(semAdd);
    return headProcQ(temp->s_next->s_procQ);
}


