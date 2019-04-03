#include "shStruct.h"
#include <stdio.h>

#define DEBUG 1 // this MAY break the code and create



char queInit(struct que* q)
{
    q->iBeg=0;
    q->iEnd=0;
    q->iCount=0;
    int i=0;
    for(;i<QUE_SIZE;i++)
        q->iQue[i]=0;
    q->bRdy=true;
    return true;
}

char queAdd(struct que* q,int input)
{
    if((q->bRdy== false ) || (q->iCount>=QUE_SIZE) ) return false;
    q->iQue[q->iEnd]=input;
    q->iEnd++;
    if(q->iEnd==QUE_SIZE)
        q->iEnd=0;
    q->iCount++;
    #ifdef DEBUG
    printf("%d>Added to Que (%d)\n",getpid(),input);
    #endif // DEBUG


    return true;
}

int queGet(struct que* q)
{
    if((q->bRdy== false ) || (q->iCount<=0) ) return 0;
    int out=q->iQue[q->iBeg];
    q->iQue[q->iBeg]=0;
    q->iBeg++;
    if(q->iBeg==QUE_SIZE)
        q->iBeg=0;
    q->iCount--;

    #ifdef DEBUG
    printf("%d>Returned from  Que (%d) number of items in Q=%d\n",getpid(),out,q->iCount);
    char huehue[1000];
    queToString(q,huehue);
    printf("%s",huehue);
    #endif // DEBUG
    return out;
}

int queGetCount(struct  que* q)
{
    if((q->bRdy== false )) return -1;
    return q->iCount;
}

void queToString(struct  que* q,char * chBuff)
{
    if(q->bRdy==false)
    {
        sprintf(chBuff,"Que not ready\n");
        return;
    }
    char chTmp[20];
    sprintf(chBuff,"iCount=%d; iBeg=%d;iEnd=%d\n",q->iCount,q->iBeg,q->iEnd);
    int i;
    for(i=0;i<QUE_SIZE;i++)
    {
        sprintf(chTmp,"%d>%d\n",i,q->iQue[i]);
        strcat(chBuff,chTmp);
    }
}

char shMemStInit(struct shMemSt * st)
{

    st->bGsleep=false;
    st->bGwake=false;
    st->bNewClient=false;
    st->invitedPID=0;
    st->iSeat=0;
    queInit(&(st->wRoom));
    return true;
}


void initRandomSleep()
{
    srand(time(NULL)*getpid());
}

void randomUsleep()
{
    randomUsleepT(1);
}
void randomUsleepT(int times)
{
    /*
    if(times<=0) times=1;
    int iRdn=rand()%10+5;
    struct timespec time;
    time.tv_sec=0;
    time.tv_nsec=1000*iRdn*times;
    nanosleep(&time,NULL);
    */
}

