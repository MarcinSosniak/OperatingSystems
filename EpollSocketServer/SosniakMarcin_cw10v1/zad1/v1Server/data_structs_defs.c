#include "data_structs_defs.h"

void clearMessage(message* me)
{
    if(me==NULL) return;
    me->m_type=EMPTY_MESSAGE;
    me->m_length=0;
    me->m_operationId=-1;
    int i=0;
    for(i=0;i<MAX_MESSAGE_LENGHT;i++)
        me->m_message[i]=0;
}

