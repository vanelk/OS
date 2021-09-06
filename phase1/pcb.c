#include "../h/const.h"
#include "../h/types.h"
#include "../h/pcb.h"
HIDDEN pcb_PTR pcb_free_h;
void debugMg(int a, int b, int c)
{
    int k = 43;
    k++;
}
/*
*
*
*/
void freePcb(pcb_PTR p)
{
    insertProcQ(&(pcb_free_h), p);
}

/*
*
*
*/
pcb_PTR allocPcb()
{
    pcb_PTR removed = removeProcQ(&(pcb_free_h));
    if(removed!=NULL){
        removed->p_child = NULL;
        removed->p_next = NULL;
        removed->p_prnt = NULL;
        removed->p_semAdd = NULL;
        removed->p_sib = NULL;
    }
    
    return removed;

}

/*
*
*
*/
void initPcbs()
{
    static pcb_t foo[MAXPROC];
    pcb_free_h = NULL;
    int i = 0;
    for (i = 0; i < MAXPROC; i++)
    {
        freePcb(&(foo[i]));
    }
}

/*
*
*
*/
pcb_PTR mkEmptyProcQ()
{
    return NULL;
}

/*
*
*
*/
int emptyProcQ(pcb_PTR tp)
{
    return (tp == NULL);
}

/*
*
*
*/
void insertProcQ(pcb_PTR *tp, pcb_PTR p)
{
    if (emptyProcQ(*tp))
    {
        p->p_next = p;
        p->p_prev = p;
    }
    else
    {
        pcb_PTR tmp = (*tp);
        p->p_next = tmp;
        p->p_prev = tmp->p_prev;
        tmp->p_prev = p;
        p->p_prev->p_next = p;
    }
    (*tp) = p;
}
/*
*
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

        pcb_PTR removed = tail->p_prev;
        tail->p_prev = removed->p_prev;
        return removed;
    }
}

pcb_PTR outProcQ(pcb_PTR *tp, pcb_PTR p)
{
    pcb_PTR tail = *tp;
    pcb_PTR current = tail;
    while (1)
    {
        if (current == p)
        {
            if (tail == p)
            {
                *tp = p->p_next;
                p->p_next->p_prev = p->p_prev;
                p->p_prev->p_next = p->p_next;
            }
            else
            {
                p->p_next->p_prev = p->p_prev;
                p->p_prev->p_next = p->p_next;
            }
            return p;
        }
        else if (current->p_next == tail)
        {
            return NULL;
        }
        current = current->p_next;
    }
}

pcb_PTR headProcQ(pcb_PTR tp)
{
    if (emptyProcQ(tp))
    {
        return NULL;
    }
    return tp->p_prev;
}

void insertChild(pcb_PTR prnt, pcb_PTR p)
{
    if (emptyChild(prnt))
    {
        prnt->p_child = p;
        p->p_prnt = prnt;
        p->p_sib = NULL;
    }
    else
    {
        p->p_sib = prnt->p_child;
        p->p_prnt = prnt;
        prnt->p_child = p;
    }
}
int emptyChild(pcb_PTR p)
{
    return (p->p_child == NULL);
}

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
