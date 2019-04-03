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


mqd_t stMainQDesc=-1;
mqd_t stMyQDesc=-1;
char sMyQName[40];

#define MAX_LINE_SIZE 255
#define DEBUG 1
void stop();
void recSIGINT(int we)
{
    stop();

    exit(0);
}

void stop()
{
    #ifdef DEBUG
    printf("Stop called \n");
    #endif // DEBUG
    my_msg stOut;
    stOut.iType=STOP;
    sprintf(stOut.msgBuff,"%6d",getpid());
    int iRet=mq_send(stMainQDesc,(char*) &stOut,MAX_SEND_LEN,2);
    if(iRet<0)
    {
        perror("error on stop on sending STOP");
    }
    iRet=mq_close(stMainQDesc);
    if(iRet<0)
    {
        perror("error on stop closing mainQ");
    }
    iRet=mq_unlink(sMyQName);
    if(iRet<0)
    {
        perror("error on stop unlinking myQ");
    }
    mq_close(stMyQDesc);

}

void end()
{
    int iRet;
    iRet=mq_close(stMainQDesc);
    if(iRet<0)
    {
        perror("error on forced shutdown on clsoing mainQ");
    }
    iRet=mq_unlink(sMyQName);
    if(iRet<0)
    {
        perror("error on forced shutdown on unlinking myQ");
    }
    mq_close(stMyQDesc);
    exit(0);
}


int sendMirror(char * input)
{
    my_msg stMessage;
    stMessage.iType=MIRROR;
    sprintf(stMessage.msgBuff,"%6d %s",getpid(),input);
    int iRet=mq_send(stMainQDesc,(char*)(&stMessage),MAX_SEND_LEN,2);
    if(iRet<0) return -9;

     iRet=mq_receive(stMyQDesc,(char*)(&stMessage),MAX_COMM_LEN,NULL);
    if(iRet<0) return -10;
    if(stMessage.iType==END) end();
    printf("%s\n",stMessage.msgBuff);
    return 0;
}

int sendCalc(char * input)
{
    my_msg stMessage;
    stMessage.iType=CALC;
    sprintf(stMessage.msgBuff,"%6d %s",getpid(),input);
    int iRet=mq_send(stMainQDesc,(char*)(&stMessage),MAX_SEND_LEN,2);
    if(iRet<0) return -11;

    iRet=mq_receive(stMyQDesc,(char*)(&stMessage),MAX_COMM_LEN,NULL);
    if(iRet<0) return -12;
    if(stMessage.iType==END) end();
    printf("%s\n",stMessage.msgBuff);
    return 0;
}

int sendTime()
{
    my_msg stMessage;
    stMessage.iType=TIME;
    sprintf(stMessage.msgBuff,"%6d",getpid());
    int iRet=mq_send(stMainQDesc,(char*)(&stMessage),MAX_SEND_LEN,2);
    if(iRet<0) return -11;

    iRet=mq_receive(stMyQDesc,(char*)(&stMessage),MAX_COMM_LEN,NULL);
    if(iRet<0) return -12;
    if(stMessage.iType==END) end();
    printf("%s\n",stMessage.msgBuff);
    return 0;

}

int sendEnd()
{
    my_msg stMessage;
    stMessage.iType=END;
    int iRet=mq_send(stMainQDesc,(char*)(&stMessage),MAX_SEND_LEN,2);
    if(iRet<0) return -11;
    return 0;
}


int live()
{
    int iRet;
    sprintf(sMyQName,"/QueTest%6d",getpid());
    struct mq_attr stCtrl;
    stCtrl.mq_flags=0;
    stCtrl.mq_msgsize=MAX_COMM_LEN-1;
    stCtrl.mq_maxmsg=MAX_MESSAGE_COUNT;
    stMyQDesc=mq_open(sMyQName,O_RDONLY | O_CREAT | O_EXCL, S_IRWXO | S_IRWXG | S_IRWXU,&stCtrl); //opening to read only
    if(stMyQDesc<0)
    {
        return -1;
    }
    my_msg stMessage;
    stMessage.iType=START;
    sprintf(stMessage.msgBuff,"%6d",getpid());
    printf("My 6 digit PID is %s\n",stMessage.msgBuff);
    sleep(1);
    stMainQDesc=mq_open(MAIN_QUE_NAME,O_WRONLY);
    if(stMainQDesc<0)
    {
        return -2;
    }
    iRet=mq_send(stMainQDesc,(char*)&stMessage,MAX_SEND_LEN,2);
    if(iRet<0)
    {
        return -3;
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
        if(k==MAX_LINE_SIZE)
        {
            k--;
            line[k]='\0';
        }
        #ifdef DEBUG
        printf("line Len:%d Line>%s\n",iLineLen,line);
        #endif

        // check if we didn't get order to quit;
        // make no block;
        #ifdef DEBUG
        printf("before getAttr \n");
        #endif // DEBUG
        iRet=mq_getattr(stMyQDesc,&stCtrl);
        if(iRet<0) return -4;
        stCtrl.mq_flags=O_NONBLOCK;
        #ifdef DEBUG
        printf("before setAttr \n");
        #endif // DEBUG
        iRet=mq_setattr(stMyQDesc,&stCtrl,NULL);
        if(iRet<0) return -5;
        #ifdef DEBUG
        printf("before random RECIEVE\n");
        #endif // DEBUG
        iRet=mq_receive(stMyQDesc,(char*)(&stMessage),MAX_COMM_LEN,NULL);
        #ifdef DEBUG
        printf("before after RECIEVE\n");
        #endif // DEBUG
        if(iRet>=0)
        {
            end();
        }
        else
        {
            #ifdef DEBUG
            perror("");
            printf("errno:%d and ret:%d\n",errno,iRet);
            #endif // DEBUG
        }

        // mak block
        iRet=mq_getattr(stMyQDesc,&stCtrl);
        if(iRet<0) return -7;
        stCtrl.mq_flags=0;
        iRet=mq_setattr(stMyQDesc,&stCtrl,NULL);
        if(iRet<0) return -8;
        //parse line


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
    return 0;

}



int main()
{
    signal(SIGINT,recSIGINT);
    int iRet=live();
    if(iRet<0)
    {
        printf("iRet=%d,errno=%d\n",iRet,errno);
        perror("");
    }
    printf("Hello world!\n");
    return 0;
}
