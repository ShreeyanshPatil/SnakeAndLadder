#include "../inc/DoublyList.h"
void GenericInsert(struct Node *p_beg, struct Node *p_mid, struct Node *p_end)
{
    p_mid->pNext = p_end;
    p_mid->pPrev = p_beg;
    p_beg->pNext = p_mid;
    p_end->pPrev = p_mid;
}