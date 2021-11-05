/**
 * PCB module Process Control Block 
 * Provides Queue Services
*/
#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
HIDDEN pcb_PTR pcb_free_h;

/*
* initialize the pcbFree list to contain all the elements of the
* static array of MAXPROC pcbs. This method will be called only
* once during data structure initialization.
*/
void initPcbs()
{
    static pcb_t pcbs[MAXPROC];
    pcb_free_h = NULL;
    int i = 0;
    for (i = 0; i < MAXPROC; i++)
    {
        freePcb(&(pcbs[i]));
    }
}

/************************************************************************/
/************************* Main Methods *********************************/
/************************************************************************/

/*
* Insert the pcb pointed to by p into the process queue whose tail-
* pointer is pointed to by tp. Note the double indirection through tp
* to allow for the possible updating of the tail pointer as well.
*/
void insertProcQ(pcb_PTR *tp, pcb_PTR p)
{
    if(!emptyProcQ(p)){
    if (emptyProcQ(*tp))
    {
        p->p_next = p;
        p->p_prev = p;
    }
    else
    {
        pcb_PTR temp = (*tp);
        p->p_next = temp;
        p->p_prev = temp->p_prev;
        temp->p_prev = p;
        p->p_prev->p_next = p;
    }
    (*tp) = p;
    }
}

/*
* Remove the first element from the process queue whose
* tail-pointer is pointed to by tp. Note the double indirection through tp
* to allow for the possible updating of the tail pointer as well.
*/
pcb_PTR removeProcQ(pcb_PTR *tp)
{
    if (emptyProcQ(*tp))
    {
        return NULL;
    }
    pcb_PTR tail = *tp;
    if (tail->p_prev == tail)
    {
        (*tp) = NULL;
        return tail;
    }
    else
    {

        pcb_PTR remove = tail->p_prev;
        remove->p_prev->p_next = remove->p_next;
        remove->p_next->p_prev = remove->p_prev;
        return remove;
    }
}

/*
* Remove the pcb pointed to by p from the process queue whose tail-
* pointer is pointed to by tp. Update the process queue's tail pointer if
* necessary. If the desired entry is not in the indicated queue
* return NULL; otherwise, return p. Note that p can point
* to any element of the process queue.
*/
pcb_PTR outProcQ(pcb_PTR *tp, pcb_PTR p)
{
    pcb_PTR tail = *tp;
    if(emptyProcQ(p)) return NULL;
    if(emptyProcQ(tail)) return NULL;
    pcb_PTR temp = tail->p_next;
    while(temp != p && temp != tail)
    {
        temp = temp->p_next;
    }
    if(temp == p){
        p->p_next->p_prev = p->p_prev;
        p->p_prev->p_next = p->p_next;
        if(temp == tail && tail->p_next == tail)
        {
            (*tp) = NULL;
        }else if(temp == tail)
        {
            (*tp) = tail->p_next;
        }
        return temp;
    } else {
        return NULL;
    }
}

/*
* Make the pcb pointed to by p a child of the pcb pointed to by parent
*/
void insertChild(pcb_PTR parent, pcb_PTR p)
{
    if(!emptyProcQ(p)){
    if (emptyChild(parent))
    {
        parent->p_child = p;
        p->p_prnt = parent;
        p->p_sib = NULL;
    }
    else
    {
        p->p_sib = parent->p_child;
        p->p_prnt = parent;
        parent->p_child = p;
    }
}
}

/*
* Make the first child of the pcb pointed to by p no longer a child of
* p. Return NULL if initially here were no children of p. Otherwise,
* return a pointer to this removed first child pcb.
*/
pcb_PTR removeChild(pcb_PTR p)
{
    if (emptyChild(p))
    {
        return NULL;
    }
    else
    {
        pcb_PTR child = p->p_child;
        p->p_child = child->p_sib;
        return child;
    }
}

/*
* Make the pcb pointed to by p no longer the child of its parent. If
* the pcb pointed to by p has no parent, return NULL; otherwise, return
* p. Note that the element pointed to by p need not be the first child of
* its parent.
*/
pcb_PTR outChild(pcb_PTR p)
{
    if(p->p_prnt == NULL) return NULL;
    pcb_PTR parent = p->p_prnt;
    pcb_PTR currentChild = parent->p_child;
    pcb_PTR lastChild = NULL;
    if (currentChild == p)
    {
        return removeChild(parent);
    }
    while (currentChild != NULL)
    {
        if (currentChild == p)
        {
            lastChild->p_sib = p->p_sib;
            p->p_prnt = NULL;
            return p;
        }
        lastChild = currentChild;
        currentChild = currentChild->p_sib;
    }
    return NULL;
}

/************************************************************************/
/************************ Helper Methods ********************************/
/************************************************************************/

/*
* Return TRUE if the queue whose tail is pointed to by tp is empty.
* Return FALSE otherwise.
*/
int emptyProcQ(pcb_PTR tp)
{
    return (tp == NULL);
}

/*
* Return a pointer to the first pcb from the process queue whose tail
* is pointed to by tp. Do not remove this pcb from the process queue.
* Return NULL if the process queue is empty.
*/
pcb_PTR headProcQ(pcb_PTR tp)
{
    if (emptyProcQ(tp))
    {
        return NULL;
    }
    return tp->p_prev;
}

/*
* Return TRUE if the pcb pointed to by p has no children. Return
* FALSE otherwise.
*/
int emptyChild(pcb_PTR p)
{
    return (p->p_child == NULL);
}

/*
* This method is used to initialize a variable to be tail pointer to a
* a process queue.
* Return a pointer to the tail of an empty process queue; i.e. NULL.
*/
pcb_PTR mkEmptyProcQ()
{
    return NULL;
}

/*
* Insert the element pointed to by p onto the pcbFree list.
*/
void freePcb(pcb_PTR p)
{
    insertProcQ(&(pcb_free_h), p);
}

/*
* Return NULL if the pcbFree list is empty. Otherwise, remove
* an element from the pcbFree list, provide initial values for ALL
* of the pcbs fields and then return a pointer
* to the removed element. pcbs get reused, so it is important that
* no previous value persist in a pcb when i gets reallocated.
*/
pcb_PTR allocPcb()
{
    pcb_PTR allocate = removeProcQ(&(pcb_free_h));
    if(allocate!=NULL){
        allocate->p_child = NULL;
        allocate->p_next = NULL;
        allocate->p_prnt = NULL;
        allocate->p_semAdd = NULL;
        allocate->p_sib = NULL;
        allocate->p_time = NULL;
        allocate->p_supportStruct = NULL;
    }
    
    return allocate;

}



