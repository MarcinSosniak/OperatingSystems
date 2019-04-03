#include "QueCommon.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>


#define MAX_LINE_SIZE 100
#define DEBUG 1

int iMainQ=-1;
int iMyQ=-1;
int iPid=-1;
my_msg stMyMessageB;

int die();
void  fSIGINT(int sigNr)
{
    int iSystemCallRet;
    stMyMessageB.iType=STOP;
    sprintf(stMyMessageB.msgBuff,"%6d",iPid);
    iSystemCallRet=msgsnd(iMainQ,&stMyMessageB,MAX_COMM_LEN,0);
    if(iSystemCallRet<0)
    {
        if(errno==EIDRM && errno==EINVAL) die();
        printf("Error on sending STOP message(other than EIDRM) errno:%d \n",errno);
    }
    die();

}


int sendMirror(char* input)
{
    int iSystemCallRet;
    sprintf(stMyMessageB.msgBuff,"%6d %s",iPid,input);
    stMyMessageB.iType=MIRROR;
    iSystemCallRet=msgsnd(iMainQ,&stMyMessageB,MAX_COMM_LEN,0);
    if(iSystemCallRet<0)
    {
        if(errno==EIDRM && errno==EINVAL) die();
        return -5;
    }



    iSystemCallRet=msgrcv(iMyQ,&stMyMessageB,MAX_COMM_LEN,0,0);
    if(iSystemCallRet<0)
    {
        return -6;
    }
    printf("%s\n",stMyMessageB.msgBuff);

    return 0;
}

int sendCalc(char* input)
{
    int iSystemCallRet;
    sprintf(stMyMessageB.msgBuff,"%6d %s",iPid,input);
    stMyMessageB.iType=CALC;
    iSystemCallRet=msgsnd(iMainQ,&stMyMessageB,MAX_COMM_LEN,0);
    if(iSystemCallRet<0)
    {
        if(errno==EIDRM && errno==EINVAL) die();
        return -7;
    }



    iSystemCallRet=msgrcv(iMyQ,&stMyMessageB,MAX_COMM_LEN,0,0);
    if(iSystemCallRet<0)
    {
        return -8;
    }
    printf("%s\n",stMyMessageB.msgBuff);

    return 0;
}

int sendTime()
{
    int iSystemCallRet;
    sprintf(stMyMessageB.msgBuff,"%6d",iPid);
    stMyMessageB.iType=TIME;
    iSystemCallRet=msgsnd(iMainQ,&stMyMessageB,MAX_COMM_LEN,0);
    if(iSystemCallRet<0)
    {
        if(errno==EIDRM && errno==EINVAL) die();
        return -9;
    }



    iSystemCallRet=msgrcv(iMyQ,&stMyMessageB,MAX_COMM_LEN,0,0);
    if(iSystemCallRet<0)
    {
        return -10;
    }
    printf("%s\n",stMyMessageB.msgBuff);

    return 0;

}

int die()
{
    struct msqid_ds stCtrlBuf;
    printf("\nExiting (Possible causes: ctrl+C, server connection error) \n");
    msgctl(iMyQ,IPC_RMID,&stCtrlBuf);
    exit(0);
    return 0;

}

int sendEnd()
{
    int iSystemCallRet;
    struct msqid_ds stCtrlBuf;
    stMyMessageB.iType=END;
    iSystemCallRet=msgsnd(iMainQ,&stMyMessageB,MAX_COMM_LEN,0);
    if(iSystemCallRet<0)
    {
        return -11;
    }
    msgctl(iMyQ,IPC_RMID,&stCtrlBuf);
    return 0;

}

int sendStop()
{
    int iSystemCallRet;
    struct msqid_ds stCtrlBuf;
    sprintf(stMyMessageB.msgBuff,"%6d",iPid);
    stMyMessageB.iType=STOP;
    iSystemCallRet=msgsnd(iMainQ,&stMyMessageB,MAX_COMM_LEN,0);
    if(iSystemCallRet<0)
    {
        return -12;
    }

    msgctl(iMyQ,IPC_RMID,&stCtrlBuf);
    return 0;

}




int live()
{
    iPid=getpid();
    int iRet;
    const char* homedir=getenv("HOME");
    key_t keyMain = ftok(homedir,PROJ);
    int iSystemCallRet;
    printf("Trying to open Que wiht Key:%d\n",keyMain);
    iMainQ=msgget(keyMain,0);
    if (iMainQ <0)
    {
        return -1;
    }

    iMyQ=msgget(keyMain +iPid,IPC_EXCL | IPC_CREAT | S_IRWXU | S_IRWXG |S_IRWXO );
    if(iMyQ<0)
    {
        return -3;
    }
    printf("My Que open %d with key:%d\n",iMyQ,keyMain+iPid);

    sprintf(stMyMessageB.msgBuff,"%6d %d",iPid,keyMain+iPid);
    stMyMessageB.iType=START;
    iSystemCallRet=msgsnd(iMainQ,&stMyMessageB,MAX_COMM_LEN,0);
    if(iSystemCallRet<0)
    {
        return -4;
    }

    int iLineLen;
    char line[MAX_LINE_SIZE];
    while( true )
    {
        iLineLen=0;
        fgets(line,MAX_LINE_SIZE,stdin);
        int k;
        for(k=0;k<MAX_LINE_SIZE-1;k++)
        {
            if(line[k]=='\n')
            {
                line[k]='\0';
                break;
            }
            iLineLen++;

        }
        #ifdef DEBUG
        printf("line Len:%d Line>%s",iLineLen,line);
        #endif

        #ifdef DEBUG
        printf("line Len:%d Line>%s\n",iLineLen,line);
        #endif
        if(line[0]=='M' &&line[1]=='I' &&line[2]=='R' &&line[3]=='R' &&line[4]=='O' &&line[5]=='R')
        {

            iRet=sendMirror(&line[6]);
            if(iRet<0) return iRet;
            continue;
        }

        if(line[0]=='C'&&line[1]=='A'&&line[2]=='L'&&line[3]=='C')
        {
            int i;
            for(i=0;i<iLineLen;i++)
            {
                if(line[4+i]!=' ') break;
            }
            if(i==iLineLen)
            {
                printf("ParseError\n");
                continue;
            }
            iRet=sendCalc(&line[4+i]);
            if(iRet<0) return iRet;
            continue;
        }
        if(line[0]=='T'&&line[1]=='I'&&line[2]=='M'&&line[3]=='E')
        {
            iRet=sendTime();
            if(iRet<0) return iRet;
            continue;
        }
        if(line[0]=='E'&&line[1]=='N'&&line[2]=='D')
        {
            iRet=sendEnd();
            if(iRet<0) return iRet;
            break;

        }
    }//end while
        printf("MessageSend\n");
    return 0;
}








int main()
{
    int ret;
    signal(SIGINT,fSIGINT);
    ret=live();
    if(ret!=0)
    {

        printf("An fatal Error has occured ret:%d errno%d\n",ret,errno);
    }
    printf("Hello world!\n");
    return 0;
}
