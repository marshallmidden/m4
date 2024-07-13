/* ------------------------------------------------------------------------ */
/* The compare function. */
static int cmp(struct note *a, struct note *b)
{
    if (a->start_time == b->start_time)
    {
        if (a->delta == b->delta)
        {
            return(a->pitch - b->pitch);
        }
        return(a->delta - b->delta);
    }
    return (a->start_time - b->start_time);
}   /* End of cmp */

/* ------------------------------------------------------------------------ */
/* Sorted list returned. */
static struct note *listsort(struct note *list)
{
    struct note    *p;
    struct note    *q;
    struct note    *e;
    struct note    *tail;
    int             nmerges;
    int             psize;
    int             qsize;
    int             i;
    int             insize = 1;

    if (!list)
    {
        return (NULL);
    }
    while (1)
    {
        p = list;
        list = NULL;
        tail = NULL;
        nmerges = 0;
        while (p)
        {
            nmerges++;
            q = p;
            psize = 0;
            for (i = 0; i < insize; i++)
            {
                psize++;
                q = q->next;
                if (!q)
                {
                    break;
                }
            }
            qsize = insize;
            while (psize > 0 || (qsize > 0 && q))
            {

                if (psize == 0)
                {
                    e = q;
                    q = q->next;
                    qsize--;
                }
                else if (qsize == 0 || !q)
                {
                    e = p;
                    p = p->next;
                    psize--;
                }
                else if (cmp(p, q) <= 0)
                {
                    e = p;
                    p = p->next;
                    psize--;
                }
                else
                {
                    e = q;
                    q = q->next;
                    qsize--;
                }
                if (tail)
                {
                    tail->next = e;
                }
                else
                {
                    list = e;
                }
                tail = e;
            }
            p = q;
        }
        tail->next = NULL;
        if (nmerges <= 1)
        {
            return (list);
        }
        insize *= 2;
    }
}   /* End of listsort */

/* ------------------------------------------------------------------------ */
