#include "../h/const.h";
#include "../h/types.h";
#include "../h/pcb.h";

void freePcb(pcb_PTR p)
{

}
pcb_PTR allocPcb()
{

}
void initPcbs()
{
    
}
pcb_PTR mkEmptyProcQ()
{
    return NULL;
}
int emptyProcQ(pcb_PTR tp)
{
    return (tp == NULL);
}
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
pcb_PTR removeProcQ(pcb_PTR *tp)
{
    if (emptyProcQ(*tp))
    {
        return NULL;
    }
    else
    {
        pcb_PTR tail = *tp;
        pcb_PTR removed = tail->p_next;
        removed->p_next->p_prev = tail;
        tail->p_next = removed->p_next;
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
}