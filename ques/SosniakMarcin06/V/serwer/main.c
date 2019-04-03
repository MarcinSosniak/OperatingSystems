#include "QueCommon.h"
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


#define MAX_CLIENTS 20

//#define DEBUG 1


int iMainQ=-1;


typedef struct
{
    int m_iPID;
    int m_iQueID;

}Client;

void reactSigInt()
{
    struct msqid_ds stCtrlBuf;
    if(iMainQ!=-1)
        msgctl(iMainQ,IPC_RMID,&stCtrlBuf);
    printf("Force Killed Que\n");
    exit(-1);
}




int serverLive()
{
    int iClientCount=0;
    int i;
    int iCPID;
    int iSystemCallRet;
    Client stClients[MAX_CLIENTS];
    struct msqid_ds stCtrlBuf;
    const char* homedir=getenv("HOME");

    for(i=0;i<MAX_CLIENTS;i++)
    {
        stClients[i].m_iPID=-1;
        stClients[i].m_iQueID=-1;
    }
    key_t key=ftok(homedir,PROJ);
    if(key<0)
        printf("failue on key creation, ERRNO:%d\n",errno);
    printf("trying to open with ID: %d\n",key);
    iMainQ=msgget(key,IPC_EXCL | IPC_CREAT | S_IRWXU | S_IRWXG |S_IRWXO  );
    if (iMainQ <0)
    {
        return -1;
    }
    printf("Server Lives, Queue is open Key=%d\n",iMainQ);
    /*
    iSystemCallRet=msgctl(iMainQ,IPC_STAT,&stCtrlBuf);
    if(iSystemCallRet)
    {
        msgctl(iMainQ,IPC_RMID,&stCtrlBuf);
        return -8;
    }
    stCtrlBuf.msg_perm.mode=stCtrlBuf.msg_perm.mode | 777;
    iSystemCallRet=msgctl(iMainQ,IPC_SET,&stCtrlBuf);
    if(iSystemCallRet)
    {
        msgctl(iMainQ,IPC_RMID,&stCtrlBuf);
        return -9;
    }
    */
    my_msg stMessageB;
    char fEnd=false;
    char fQEmpty=false;
    int iRet;
    int iReadBytes;
    //printf("before While\n");
    while(!(fEnd && fQEmpty))
    {
        if(fEnd)
        {
            iRet=msgrcv(iMainQ,&stMessageB, MAX_COMM_LEN,0,IPC_NOWAIT);
        }
        else
        {

            #ifdef DEBUG
            printf("Await message\n");
            #endif // DEBUG
            iRet=msgrcv(iMainQ,&stMessageB, MAX_COMM_LEN,0,MSG_NOERROR);
            #ifdef DEBUG
            printf("Got message \n");
            #endif // DEBUG
        }
        if(iRet==-1)
        {   if(errno != ENOMSG)
            {
                printf("Errno =%d\n",errno);
                msgctl(iMainQ,IPC_RMID,&stCtrlBuf);
                return -6;
            }
            //sleep(1);
            fQEmpty=true;
            continue;
        }

        #ifdef DEBUG
        printf("RecievedMessage Type:%d, tekst:%s\n",stMessageB.iType,stMessageB.msgBuff);
        #endif // DEBUG
        iReadBytes=iRet;

        if(stMessageB.iType==START)
        {
            #ifdef DEBUG
            printf("recived START\n");
            #endif // DEBUG
            if(iClientCount==MAX_CLIENTS-1)
            {
                printf("Exceeded max client number\n");
                msgctl(iMainQ,IPC_RMID,&stCtrlBuf);
                return -2;
            }
            stMessageB.msgBuff[6]='\0';
            stClients[iClientCount].m_iPID=atoi(stMessageB.msgBuff);
            stClients[iClientCount].m_iQueID=atoi(stMessageB.msgBuff+7);
            iSystemCallRet=msgget(stClients[iClientCount].m_iQueID,0);
            //printf("\nOpened Client Q for client: %d with systemret: %d \n",stClients[iClientCount].m_iPID,iSystemCallRet);
            if(iSystemCallRet<0)
            {
                printf("Cannot open client Queue. Client pid:%d Errno:%d\n",stClients[iClientCount].m_iPID,errno);
                perror("\nPerrno:");
                printf("\n");
                msgctl(iMainQ,IPC_RMID,&stCtrlBuf);
                return -4;
            }
            stClients[iClientCount].m_iQueID=iSystemCallRet;
            iClientCount++;
        }// end if START

        if(stMessageB.iType==STOP)
        {
            #ifdef DEBUG
            printf("recived STOP");
            #endif // DEBUG
            iCPID=atoi(stMessageB.msgBuff);
            for(i=0;i<iClientCount;i++)
            {
                if(stClients[i].m_iPID==iCPID)
                    break;
            }
            if(i==iClientCount) continue;// unregistred client wants to end
            iClientCount--;
            for(;i<iClientCount;i++)
            {
                stClients[i].m_iQueID=stClients[i+1].m_iQueID;
                stClients[i].m_iPID=stClients[i+1].m_iPID;
            }

        } // end if STOP

        if(stMessageB.iType==MIRROR)
        {
            #ifdef DEBUG
            printf("recived MIRROR");
            #endif // DEBUG
            stMessageB.msgBuff[6]='\0';
            iCPID=atoi(stMessageB.msgBuff);
            for(i=0;i<iClientCount;i++)
            {
                if(stClients[i].m_iPID==iCPID)
                    break;
            }
            if(i==iClientCount) continue;// ignore unregisterd clients.
            int k;
            for(k=0;k<MAX_COMM_LEN-8;k++)
            {
                if(stMessageB.msgBuff[k+7]=='\0') break;
            }
            k--;// k+7 points to the last non \0 char
            my_msg stOut;
            stOut.iType=MIRROR;
            int l;
            for(l=0;l<=k;l++)
            {
                stOut.msgBuff[l]=stMessageB.msgBuff[k+7-l];
            }
            stOut.msgBuff[k+1]='\0';
            iSystemCallRet=msgsnd(stClients[i].m_iQueID,&stOut,MAX_COMM_LEN,0);
            if(iSystemCallRet<0)
            {
                printf("clientPid:%d,m_iQueID:%d\n",stClients[i].m_iPID,stClients[i].m_iQueID);
                msgctl(iMainQ,IPC_RMID,&stCtrlBuf);
                return -3;
            }
        }// end  if MIRROR


        if(stMessageB.iType==TIME)
        {
            #ifdef DEBUG
            printf("recived TIME");
            #endif // DEBUG
            stMessageB.msgBuff[6]='\0';
            iCPID=atoi(stMessageB.msgBuff);
            for(i=0;i<iClientCount;i++)
            {
                if(stClients[i].m_iPID==iCPID)
                    break;
            }
            if(i==iClientCount) continue;// ignore unregisterd clients.
            my_msg stOut;
            stOut.iType=TIME;
            time_t timer;
            struct tm* tm_info;
            time(&timer);
            tm_info = localtime(&timer);
            strftime(stOut.msgBuff,MAX_COMM_LEN-1,"%Y.%m.%d %H:%M:%S",tm_info);
            iSystemCallRet=msgsnd(stClients[i].m_iQueID,&stOut,MAX_COMM_LEN,0);
            if(iSystemCallRet<0)
            {
                msgctl(iMainQ,IPC_RMID,&stCtrlBuf);
                return -5;
            }
        }// end if DATE

        if(stMessageB.iType==CALC)
        {
            #ifdef DEBUG
            printf("recived CALC");
            #endif // DEBUG
            stMessageB.msgBuff[6]='\0';
            iCPID=atoi(stMessageB.msgBuff);
            for(i=0;i<iClientCount;i++)
            {
                if(stClients[i].m_iPID==iCPID)
                    break;
            }
            if(i==iClientCount) continue;// ignore unregisterd clients.
            my_msg stOut;
            stOut.iType=CALC;
            int iFop;
            int iSop;
            int iOutcome;
            int k=0;
            char chOperator;
            for(k=7;k<MAX_COMM_LEN;k++)
            {
                if(stMessageB.msgBuff[k]=='+' ||stMessageB.msgBuff[k]=='-' ||stMessageB.msgBuff[k]=='*' ||stMessageB.msgBuff[k]=='/' )
                    break;
            }
            if(k==MAX_COMM_LEN)
            {
                printf("minor error, invalid package format on CALC from %d\n",stClients[i].m_iPID);
                continue;
            }
            chOperator=stMessageB.msgBuff[k];
            stMessageB.msgBuff[k]='\0';
            iFop=atoi(&stMessageB.msgBuff[7]);
            iSop=atoi(&stMessageB.msgBuff[k+1]);
            if(chOperator=='+')
                iOutcome=iFop+iSop;
            if(chOperator=='-')
                iOutcome=iFop-iSop;
            if(chOperator=='*')
                iOutcome=iFop*iSop;
            if(chOperator=='/')
                iOutcome=iFop/iSop;
            sprintf(stOut.msgBuff,"%d",iOutcome);
            iSystemCallRet=msgsnd(stClients[i].m_iQueID,&stOut,MAX_COMM_LEN,0);
            if(iSystemCallRet<0)
            {
                msgctl(iMainQ,IPC_RMID,&stCtrlBuf);
                return -7;
            }
        }//end if CALC
        if(stMessageB.iType==END)
        {
            #ifdef DEBUG
            printf("recived END");
            #endif // DEBUG
            fEnd=true;
        }
    }// end while

    msgctl(iMainQ,IPC_RMID,&stCtrlBuf);

    return 0;
}

int main()
{
    signal(SIGINT,reactSigInt);
    int ret=serverLive();
    if(ret!=0)
    {
        printf("ret= %d,Errno=%d\n",ret,errno);
    }
    printf("Hello world!\n");
    return 0;
}

