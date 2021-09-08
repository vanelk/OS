#include <limits.h>
#include "../h/pcb.h"
#include "../h/asl.h"
HIDDEN semd_t *semd_h, *semFree_h;

semd_t *allocSem()
{
    if (((int*)semFree_h) == 0)
        return NULL;
    semd_t *freed = semFree_h;
    semFree_h = semFree_h->s_next;
    freed->s_next = NULL;
    freed->s_procQ = mkEmptyProcQ();
    freed->s_semAdd = NULL; 
    return freed;
}

/*
*
*
*/
void deallocSem(semd_t *sem)
{
    
    sem->s_next = semFree_h;
    semFree_h = sem;
}

semd_t *search(int *semAdd)
{
    semd_t *current = (semd_t *)semd_h;
    while (semAdd > (current->s_next->s_semAdd))
    {
        current = current->s_next;
    }

    return current;
}

/*
*
*
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
*
*
*/
int insertBlocked(int *semAdd, pcb_PTR p)
{
    semd_t *daddy = search(semAdd);
    if (daddy->s_next->s_semAdd == semAdd)
    {
        p->p_semAdd = semAdd;
        insertProcQ(&(daddy->s_next->s_procQ), p);
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
        sem->s_next = daddy->s_next;
        daddy->s_next = sem;
    }
    return FALSE;
}

/*
*
*
*/
pcb_PTR removeBlocked(int *semdAdd)
{
    semd_t *daddy = search(semdAdd);
    if (daddy->s_next->s_semAdd == semdAdd)
    {
        pcb_PTR remove = removeProcQ(&(daddy->s_next->s_procQ));
        if (remove == NULL)
        {
            return NULL;
        }
        if (emptyProcQ(daddy->s_next->s_procQ))
        {
            semd_t *removed = daddy->s_next;
            daddy->s_next = daddy->s_next->s_next;
            deallocSem(removed);
        }
        return remove;
    }
    return NULL;
}

/*
*
*
*/
pcb_PTR outBlocked(pcb_PTR p)
{
    int *semdAdd = p->p_semAdd;
    semd_t *daddy = search(semdAdd);
    if (daddy->s_next->s_semAdd == semdAdd)
    {
        pcb_PTR remove = outProcQ(&(daddy->s_next->s_procQ), p);
        if (remove == NULL)
        {
            return NULL;
        }
        
        if (emptyProcQ(daddy->s_next->s_procQ))
        {
            semd_t *removed = daddy->s_next;
            daddy->s_next = daddy->s_next->s_next;
            deallocSem(removed);
        }
        return remove;
    }
    return NULL;
}

/*
*
*
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
