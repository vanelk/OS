#include <limits.h>
#include "../h/pcb.h"
#include "../h/asl.h"
HIDDEN semd_t *semd_h, *semdFree_h = NULL;

/************************************************************************/
/************************ Hidden Methods ********************************/
/************************************************************************/

/*
* Return NULL if the semdFree list is empty. Otherwise, remove
* an element from the semdFree list, provide initial values for ALL
* of the semd's fields and then return a pointer
* to the removed element. semds get reused, so it is important that
* no previous value persist in a pcb when it gets reallocated.
*/
HIDDEN semd_t *allocSem()
{
    if (semdFree_h == NULL)
        return NULL;
    semd_t *freed = semdFree_h;
    semdFree_h = semdFree_h->s_next;
    freed->s_next = NULL;
    freed->s_procQ = mkEmptyProcQ();
    freed->s_semAdd = NULL; 
    return freed;
}

/*
* Insert the element pointed to by sem onto the semdFree list.
*/
HIDDEN void deallocSem(semd_t *sem)
{
    
    sem->s_next = semdFree_h;
    semdFree_h = sem;
}

HIDDEN semd_t *search(int *semAdd)
{
    semd_t *current = semd_h;
    while (semAdd > (current->s_next->s_semAdd))
    {
        current = current->s_next;
    }

    return current;
}

/************************************************************************/
/************************* Main Methods *********************************/
/************************************************************************/

/*
* Initialize the semdFree list to contain all the elements of the array.
* This method will be only called once during data structure initialization.
*/
void initASL()
{
    static semd_t semdTable[MAXPROC + 2];
    int i;
    for (i = 0; i < MAXPROC; i++)
    {
        deallocSem(&semdTable[i + 1]);
    }
    semd_h = (&semdTable[0]);
    semd_h->s_semAdd = (int *)INT_MIN;
    semd_h->s_next = (&semdTable[MAXPROC + 1]);
    semd_h->s_next->s_semAdd = (int *)INT_MAX;
}

/*
* Insert the pcb pointed to by p at the tail of the process queue as-
* sociated with the semaphore whose physical address is semAdd and
* set the semaphore address of p to semAdd. If the semaphore is cur-
* rently not actuve, allocate a new descriptor from the semdFree list,
* insert it in the ASL, initialize all of the fields, and proceed as
* above. If a new semaphore descriptor needs to be allocated and the
* semdFree list is empty, return TRUE. In all other cases return FALSE.
*/
int insertBlocked(int *semAdd, pcb_PTR p)
{
    semd_t *parent = search(semAdd);
    if (parent->s_next->s_semAdd == semAdd)
    {
        p->p_semAdd = semAdd;
        insertProcQ(&(parent->s_next->s_procQ), p);
    }
    else
    {
        semd_t *sem = allocSem();
        if (sem == NULL)
        {
            return TRUE;
        }
        sem->s_semAdd = semAdd;
        sem->s_procQ = mkEmptyProcQ();
        p->p_semAdd = semAdd;
        insertProcQ(&(sem->s_procQ), p);
        sem->s_next = parent->s_next;
        parent->s_next = sem;
    }
    return FALSE;
}

/*
* Search the ASL for a descriptor of this semaphore. If none is
* found, return NULL; otherwise, remove the first pcb from
* the process queue of the found semaphore descriptor and return a
* pointer to it. If the process queue for this semaphore becomes empty
* remove the semaphore descriptor from the ASL and return it to the
* semdFree list.
*/
pcb_PTR removeBlocked(int *semdAdd)
{
    semd_t *parent = search(semdAdd);
    if (parent->s_next->s_semAdd == semdAdd)
    {
        pcb_PTR remove = removeProcQ(&(parent->s_next->s_procQ));
        if (remove == NULL)
        {
            return NULL;
        }
        if (emptyProcQ(parent->s_next->s_procQ))
        {
            semd_t *removed = parent->s_next;
            parent->s_next = parent->s_next->s_next;
            deallocSem(removed);
        }
        return remove;
    }
    return NULL;
}

/*
* Remove the pcb pointed to by p from the process queue associated
* with p's semaphore on the ASL. if pcb pointed
* to by p does not appear in the process queue associated with p's
* semaphore, which is an error condition, return NULL; otherwise,
* return p.
*/
pcb_PTR outBlocked(pcb_PTR p)
{
    int *semdAdd = p->p_semAdd;
    semd_t *parent = search(semdAdd);
    if (parent->s_next->s_semAdd == semdAdd)
    {
        pcb_PTR remove = outProcQ(&(parent->s_next->s_procQ), p);
        if (remove == NULL)
        {
            return NULL;
        }
        
        if (emptyProcQ(parent->s_next->s_procQ))
        {
            semd_t *removed = parent->s_next;
            parent->s_next = parent->s_next->s_next;
            deallocSem(removed);
        }
        return remove;
    }
    return NULL;
}

/*
* Return a pointer to the pcb that is at the head of the process queue
* associated with the semaphore semAdd. Return NULL if semAdd is
* not found on the ASL or if the process queue associated with semAdd
* is empty.
*/
pcb_PTR headBlocked(int *semAdd)
{
    semd_t *temp = search(semAdd);
    if (temp->s_next->s_semAdd == semAdd)
    {
        return headProcQ(temp->s_next->s_procQ);
    }
    return NULL;
}
