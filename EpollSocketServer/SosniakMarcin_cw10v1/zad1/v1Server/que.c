#include "que.h"
#include "stdio.h"
#include "string.h"
//#define NULL 0


char msQueIsFull(messageQue* q)
{
    if(q->iCount>=q->iMaxSize) return true;
    return false;
}


char msQueInit(messageQue* q)
{
    q->iMaxSize=QUE_SIZE;
    q->iBeg=0;
    q->iEnd=0;
    q->iCount=0;
    int i=0;
    for(;i<q->iMaxSize;i++)
        clearMessage(q->iQue + i);
    q->bRdy=true;
    return true;
}

char msQueAdd(messageQue* q,message input)
{
    if((q->bRdy== false ) || (q->iCount>=q->iMaxSize) ) return false;
    q->iQue[q->iEnd]=input;
    q->iEnd++;
    if(q->iEnd==q->iMaxSize)
        q->iEnd=0;
    q->iCount++;

    return true;
}

message msQueGet(messageQue* q)
{
    message out;
    clearMessage(&out);
    if((q->bRdy== false ) || (q->iCount<=0) ) return out;
    out=q->iQue[q->iBeg];
    clearMessage(&(q->iQue[q->iBeg]));
    q->iBeg++;
    if(q->iBeg==q->iMaxSize)
        q->iBeg=0;
    q->iCount--;
    return out;
}

int msQueGetCount(messageQue* q)
{
    if((q->bRdy== false )) return -1;
    return q->iCount;
}

void msQueToString(messageQue* q,char * chBuff)
{
    if(q->bRdy==false)
    {
        sprintf(chBuff,"Que not ready\n");
        return;
    }
    char chTmp[60+MAX_MESSAGE_LENGHT];
    sprintf(chBuff,"iCount=%d; iBeg=%d;iEnd=%d\n",q->iCount,q->iBeg,q->iEnd);
    int i;
    for(i=0;i<q->iMaxSize;i++)
    {
        if(q->iQue[i].m_type==EMPTY_MESSAGE)
            sprintf(chTmp,"%d>EMPTY_MESSAGE\n",i);
        else
        {
            sprintf(chTmp,"%d>type:%d ; m_length:%d ; operationID %d\n",i,q->iQue[i].m_type,q->iQue[i].m_length,q->iQue[i].m_operationId);
        }
        strcat(chBuff,chTmp);
    }
}


/*
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
                TASK QUE
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
*/


char tQueIsFull(taskQue* q)
{
    if(q->iCount>=q->iMaxSize) return true;
    return false;
}


char tQueInit(taskQue* q)
{
    q->iMaxSize=QUE_SIZE;
    q->iBeg=0;
    q->iEnd=0;
    q->iCount=0;
    int i=0;
    for(;i<q->iMaxSize;i++)
        clearTask(q->iQue + i);
    q->bRdy=true;
    return true;
}

char tQueAdd(taskQue* q,task input)
{
    if((q->bRdy== false ) || (q->iCount>=q->iMaxSize) ) return false;
    q->iQue[q->iEnd]=input;
    q->iEnd++;
    if(q->iEnd==q->iMaxSize)
        q->iEnd=0;
    q->iCount++;

    return true;
}

task tQueGet(taskQue* q)
{
    task out;
    clearTask(&out);
    if((q->bRdy== false ) || (q->iCount<=0) ) return out;
    out=q->iQue[q->iBeg];
    clearTask(&(q->iQue[q->iBeg]));
    q->iBeg++;
    if(q->iBeg==q->iMaxSize)
        q->iBeg=0;
    q->iCount--;
    return out;
}

int tQueGetCount(taskQue* q)
{
    if((q->bRdy== false )) return -1;
    return q->iCount;
}

void tQueToString(taskQue* q,char * chBuff)
{
    if(q->bRdy==false)
    {
        sprintf(chBuff,"Que not ready\n");
        return;
    }
    char chTmp[60+MAX_MESSAGE_LENGHT];
    sprintf(chBuff,"iCount=%d; iBeg=%d;iEnd=%d\n",q->iCount,q->iBeg,q->iEnd);
    int i;
    for(i=0;i<q->iMaxSize;i++)
    {
        if(q->iQue[i].m_bOperation==EMPTY_OPERATOR)
            sprintf(chTmp,"%d>EMPTY_MESSAGE\n",i);
        else
        {
            sprintf(chTmp,"%dOperation>:%d ; arg1:%f ; arg2 %f ; iTaskId :%d\n",i,q->iQue[i].m_bOperation,q->iQue[i].m_dArg1,q->iQue[i].m_dArg2,q->iQue[i].m_iTaskId);
        }
        strcat(chBuff,chTmp);
    }
}




















