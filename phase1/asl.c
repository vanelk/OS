#include "../h/pcb.h";
#include "../h/asl.h";
HIDDEN semd_t *semd_h, *semFree_h;

/*
*
*
*/
void insertSem(semd_t * sem){
    semd_t *next = semFree_h;
    semFree_h->s_next = next;
    semFree_h = next;
}

/*
*
*
*/
semd_t *allocSem(){
    semd_t *freed = semFree_h;
    semFree_h = semFree_h->s_next;
    return freed;
}

/*
*
*
*/
semd_t *deallocSem(semd_t *sem){
    sem->s_next = semFree_h;
    semFree_h = sem;
}

semd_t *search(int *semAdd){
    semd_t *current = semd_h;
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
    for(int i=0; i<MAXPROC; i++){
        deallocSem(&semdTable[i]);
    }
}

/*
*
*
*/
int insertBlocked(int *semAdd, pcb_PTR p)
{
    semd_t *currentS = semd_h;
    int found, inList;
    do
    {
        found = (currentS->s_semAdd == semAdd);
        inList = (currentS->s_semAdd < semAdd);
        if (found) {
            insertProcQ(&(currentS->s_procQ), p);
        }
        if (inList){
            currentS = currentS->s_next;
        }
    } while (!found || !inList);
    if(!inList){
        /*allocate from the semdFree list;*/ 
        semd_t *sem = allocSem();
        sem->s_semAdd = semAdd;
        sem->s_procQ = mkEmptyProcQ();
        sem->s_next = currentS->s_next;
        currentS->s_next = sem;

    }
}

/*
*
*
*/
pcb_PTR removeBlocked(int *semdAdd){
    semd_t *prev = NULL;
    semd_t *currentS = semd_h;
    while(currentS->s_semAdd <= semdAdd){
        if(currentS == semdAdd){
            return removeProcQ(&(currentS->s_procQ));
            if(emptyProcQ(&(currentS->s_procQ))){
                if(prev != NULL){
                    prev->s_next = currentS->s_next;
                } else {
                    semd_h = semd_h->s_next;
                }
                deallocSem(currentS);
            }
        }
        prev = currentS;
        currentS = currentS->s_next;
    }
    return NULL;
}

/*
*
*
*/
pcb_PTR outBlocked(pcb_PTR p){
    semd_t *parent = search(p->p_semAdd);

    if(parent->s_semAdd == MAXINT){
	return NULL;
    }

    if(parent->s_next->s_semAdd == p->p_semAdd){
        pcb_PTR pcbToReturn = outProcQ(&(parent->s_next->s_procQ), p);

	if(emptyProcQ(&(parent->s_next->s_procQ)){
	    deallocSem(parent->s_next)
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
pcb_PTR headBlocked(int semAdd){
    semd_t *temp = search(semAdd)
    return headProcQ(temp->s_next->s_proqQ);
}


