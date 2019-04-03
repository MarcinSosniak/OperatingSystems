#include "QueCommonPOSIX.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>



#define MAX_CLIENTS 20






#define SHOW 1



mqd_t stMainQDesc=-1;
char sMainQName[40];

typedef struct{
    int m_iPid;
    mqd_t m_stQueDesc;
}Client;

Client stClients[MAX_CLIENTS];
int iClientCount;


void die()
{
    printf("---------IN DIE()--------\n");
    int i=0;
    int iRet;
    my_msg stOut;
    stOut.iType=END;
    for(i=0;i<iClientCount;i++)
    {
        iRet=mq_send(stClients[i].m_stQueDesc,(char *)(&stOut),MAX_SEND_LEN,1);
        #ifdef SHOW
            printf("send END to: %d\n",stClients[i].m_iPid);
            if(iRet<0)
            {
                perror("");
            }
        #endif // SHOW
    }
    mq_unlink(sMainQName);
    mq_close(stMainQDesc);
    exit(0);
}


void reactSIGINT(int we)
{
    die();

}





int recSTART(my_msg * stReadM)
{
    int iSysRet;
    if(iClientCount>=MAX_CLIENTS-1)
    {
        printf("Warning: Maximum Clients exceeded\n");
        return 0;
    }
    stClients[iClientCount].m_iPid=atoi(stReadM->msgBuff);
    if(stClients[iClientCount].m_iPid==0)
    {
        printf("Warning: Invalid Message format on START recieved \n");
        return 0;
    }
    char sQueueName[40];
    sprintf(sQueueName,"/QueTest%6d",stClients[iClientCount].m_iPid);
    iSysRet=mq_open(sQueueName,O_WRONLY);
    if(iSysRet<0)
    {
        return -3;
    }

    stClients[iClientCount].m_stQueDesc=iSysRet;

    #ifdef SHOW
    printf("new start from PID=%d\n",stClients[iClientCount].m_iPid);
    #endif // SHOw
    iClientCount++;
    return 0;
}

int recMIRROR(my_msg * stReadM)
{

    stReadM->msgBuff[6]='\0';
    my_msg stWriteM;
    int iCpid=atoi(stReadM->msgBuff);
    int i;
    for(i=0;i<iClientCount;i++)
    {
        if(stClients[i].m_iPid==iCpid)
            break;
    }
    if(i==iClientCount)
    {
        printf("Warning: Recived non-START command(MIRROR) from unregistered client, PID:%d\n",iCpid);
        return 0;
    }
    for(int iCounter1=0;iCounter1<MAX_COMM_LEN;iCounter1++)
    {
        stWriteM.msgBuff[iCounter1]='\0';
    }
    int k;
    for(k=0;k+7<MAX_SEND_LEN;k++)
    {
        if(stReadM->msgBuff[k+7]=='\0')
            break;
    }
    k--;
    int l;
    for(l=0;l<=k;l++)
    {
        stWriteM.msgBuff[l]=stReadM->msgBuff[k+7-l];
    }
    stWriteM.msgBuff[l+1]='\0';
    #ifdef SHOW
    printf("MIRROR BUFFTOSEND:\'%s\'\n",stWriteM.msgBuff);
    #endif // SHOW
    stWriteM.iType=MIRROR;
    int iSystemRet=mq_send(stClients[i].m_stQueDesc,(char*)(&stWriteM),MAX_SEND_LEN,2);
    if(iSystemRet<0)
    {
        return -3;
    }
    return 0;
}

int recSTOP(my_msg * stReadM)
{
    stReadM->msgBuff[6]='\0';
    int iCpid=atoi(stReadM->msgBuff);
    int i;
    for(i=0;i<iClientCount;i++)
    {
        if(stClients[i].m_iPid==iCpid)
            break;
    }
    if(i==iClientCount)
    {
        printf("Warning: Recived non-START command(STOP) from unregistered client, PID:%d; ignoring\n",iCpid);
        return 0;
    }
    #ifdef SHOW
    printf("STOPPED from PID=%d\n",stClients[i].m_iPid);
    #endif // SHOW
    int iRet=mq_close(stClients[i].m_stQueDesc);
    if(iRet<0)
    {
        perror("failed closing on STOP message");
    }

    iClientCount--;
    for(;i<iClientCount;i++)
    {
        stClients[i].m_iPid=stClients[i+1].m_iPid;
        stClients[i].m_stQueDesc=stClients[i+1].m_stQueDesc;
    }

    return 0;
}
int recEND(struct mq_attr * stCtrl,char * fEnd)
{
    *fEnd=true;
    int iSystemRet;
    if(stCtrl->mq_flags != O_NONBLOCK)
    {
        iSystemRet=mq_getattr(stMainQDesc,stCtrl);
        if(iSystemRet < 0)
            return -401;
        stCtrl->mq_flags=O_NONBLOCK;
        iSystemRet=mq_setattr(stMainQDesc,stCtrl,NULL);
    }
    if(iSystemRet<0)
    {
        return -4;
    }
    return 0;
}
int recTIME(my_msg * stReadM)
{
    int iSystemCallRet;
    stReadM->msgBuff[6]='\0';
    my_msg stWriteM;
    int iCpid=atoi(stReadM->msgBuff);
    int i;
    for(i=0;i<iClientCount;i++)
    {
        if(stClients[i].m_iPid==iCpid)
            break;
    }
    if(i==iClientCount)
    {
        printf("Warning: Recived non-START command(TIME) from unregistered client, PID:%d\n",iCpid);
        return 0;
    }

    time_t timer;
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    strftime(stWriteM.msgBuff,MAX_SEND_LEN-2,"%Y.%m.%d %H:%M:%S",tm_info);
    iSystemCallRet=mq_send(stClients[i].m_stQueDesc,(char*)(&stWriteM),MAX_SEND_LEN,2);
    if(iSystemCallRet<0)
    {
        return -5;
    }

    return 0;
}
int recCALC(my_msg * stReadM)
{
    int iSystemCallRet;
    stReadM->msgBuff[6]='\0';
    my_msg stWriteM;
    int iCpid=atoi(stReadM->msgBuff);
    int i;
    for(i=0;i<iClientCount;i++)
    {
        if(stClients[i].m_iPid==iCpid)
            break;
    }
    if(i==iClientCount)
    {
        printf("Warning: Recived non-START command(CALC) from unregistered client, PID:%d\n",iCpid);
        return 0;
    }

    int iFop;
    int iSop;
    int iOutcome;
    int k=0;
    char chOperator;
    for(k=7;k<MAX_COMM_LEN;k++)
    {
        if(stReadM->msgBuff[k]=='+' ||stReadM->msgBuff[k]=='-' ||stReadM->msgBuff[k]=='*' ||stReadM->msgBuff[k]=='/' )
            break;
    }
    if(k==MAX_COMM_LEN)
    {
        printf("Warning: invalid package format on CALC from %d\n",stClients[i].m_iPid);
        return 0;
    }
    chOperator=stReadM->msgBuff[k];
    stReadM->msgBuff[k]='\0';
    iFop=atoi(&stReadM->msgBuff[7]);
    iSop=atoi(&stReadM->msgBuff[k+1]);
    if(chOperator=='+')
        iOutcome=iFop+iSop;
    if(chOperator=='-')
        iOutcome=iFop-iSop;
    if(chOperator=='*')
        iOutcome=iFop*iSop;
    if(chOperator=='/')
        iOutcome=iFop/iSop;
    sprintf(stWriteM.msgBuff,"%d",iOutcome);
    stWriteM.iType=CALC;
    iSystemCallRet=mq_send(stClients[i].m_stQueDesc,(char*)&stWriteM,MAX_SEND_LEN,2);
    if(iSystemCallRet<0)
    {
        return -6;
    }


    return 0;
}




int live()
{
    sprintf(sMainQName,MAIN_QUE_NAME);
    struct mq_attr stCtrl;

    stCtrl.mq_flags=0;
    stCtrl.mq_msgsize=sizeof(my_msg)-1;
    stCtrl.mq_maxmsg=MAX_MESSAGE_COUNT;
    stMainQDesc=mq_open(sMainQName,O_RDWR | O_CREAT , S_IRWXO | S_IRWXG | S_IRWXU, &stCtrl); //opening to read only
    if(stMainQDesc<0)
    {
        return -1;
    }




    #ifdef SHOW
    printf("open Que with name \"%s\" and mqd_t:%d\n",sMainQName,stMainQDesc);
    #endif // SHOW
    my_msg stReadM;
    int iSystemRet;
    char fEnd=false;
    char fClear=false;
    while(!(fEnd && fClear))
    {
        #ifdef SHOW
        printf("before first read\n");
        #endif // SHOW
        iSystemRet=mq_receive(stMainQDesc,(char*)(&stReadM),sizeof(my_msg),NULL);
        if(iSystemRet<0)
        {
            if(fEnd)
            {
                fClear=true;
            }
            else
            {
                return -2;
            }
        }
        #ifdef SHOW
        printf("recived message as follows Type:%d, Message:%s\n",stReadM.iType,stReadM.msgBuff);
        #endif // SHOW
        switch(stReadM.iType)
        {
        case MIRROR:
            iSystemRet=recMIRROR(&stReadM);
            if(iSystemRet<0)
            {
                return iSystemRet;
            }
            break;

        case START:
            iSystemRet=recSTART(&stReadM);
            if(iSystemRet<0)
            {
                return iSystemRet;
            }
            break;

        case CALC:
            iSystemRet=recCALC(&stReadM);
            if(iSystemRet<0)
            {
                return iSystemRet;
            }
            break;

        case END:
            iSystemRet=recEND(&stCtrl,&fEnd);
            if(iSystemRet<0)
            {
                return iSystemRet;
            }
            break;

        case STOP:
            iSystemRet=recSTOP(&stReadM);
            if(iSystemRet<0)
            {
                return iSystemRet;
            }
            break;

        case TIME:
            iSystemRet=recTIME(&stReadM);
            if(iSystemRet<0)
            {
                return iSystemRet;
            }
            break;
        }
    }//end while;
    die();
    return 0;
}



int main()
{
    mq_unlink("/QueTest000005");
    mq_unlink("/QueTest000004");
    mq_unlink("/QueTest000003");
    mq_unlink("/QueTest000002");
    mq_unlink("/QueTest000001");
    mq_unlink("/QueTest000000");
    signal(SIGINT,reactSIGINT);
    int iRet=live();
    if(iRet!=0)
    {
        printf("\n\niRet=%d; errno=%d\n\n",iRet,errno);
        perror("");
    }
    printf("Hello world!\n");
    return 0;
}
