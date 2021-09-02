#include "../h/const.h";
#include "../h/types.h";
#include "../h/pcb.h";
HIDDEN pcb_PTR pcb_free_h;

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
    return removeProcQ(&(pcb_free_h));
}

/*
*
*
*/
void initPcbs()
{
    static pcb_t foo[MAXPROC];
    pcb_free_h = NULL;
    for (int i = 0; i < MAXPROC; i++)
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
        *tp = p;
    }
    else
    {
        pcb_PTR tmp = *tp;
        *tp = p;
        p->p_next = tmp->p_next;
        p->p_prev = tmp;
        tmp->p_next = p;
        p->p_next->p_prev = p;
    }
}
/* [X]*/
pcb_PTR removeProcQ(pcb_PTR *tp)
{
    if (emptyProcQ(*tp))
    {
        return NULL;
    }
    else
    {
        pcb_PTR tail = *tp;
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
    return tp->p_prev;
}

void insertChild(pcb_PTR prnt, pcb_PTR p)
{
    p->p_sib = prnt->p_child;
    p->p_prnt = prnt;
    prnt->p_child = p;
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
    pcb_PTR parent = p->p_prnt;
    pcb_PTR currentChild = parent->p_child;
    pcb_PTR lastChild = NULL;
    if (currentChild == p)
    {
        removeChild(parent);
    }
    while (1)
    {
        if (currentChild == p)
        {
            lastChild->p_sib = p->p_sib;
            return p;
        }
        else
        {
            lastChild = currentChild;
            currentChild = currentChild->p_sib;
        }
    }
}
