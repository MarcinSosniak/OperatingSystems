#include <pthread.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <signal.h>



#include "data_structs_defs.h"

#define UNIX_MODE 0
#define NET_MODE 1



#define DEBUGPRINT 1


struct sockaddr_un unixSockStruct;
struct sockaddr_in netSockStruct;

bool iCanLive=true;


void signalHldr(int iSig)
{
    iCanLive=false;
}




uint32_t parseIPV4string(char* ipAddress) {
  char ipbytes[4];
  sscanf(ipAddress, "%uhh.%uhh.%uhh.%uhh", &ipbytes[3], &ipbytes[2], &ipbytes[1], &ipbytes[0]);
  return ipbytes[0] | ipbytes[1] << 8 | ipbytes[2] << 16 | ipbytes[3] << 24;
}

void live(char* sMyName, int mySockFd);



void initUnix(char* sMyName,char* sUnixSocketName)
{
    printf("Activating client with name '%s' and unix socket name '%s'\n",sMyName,sUnixSocketName);
    int mySockFd=socket(AF_UNIX,SOCK_DGRAM ,0);
    if(mySockFd<0) {perror("error on creating my socket(unix) "); exit(-1);}
    unixSockStruct.sun_family=AF_UNIX;
    strcpy(unixSockStruct.sun_path,sUnixSocketName);
    int iRet=connect(mySockFd,(struct sockaddr*)&unixSockStruct,sizeof(struct sockaddr_un));
    printf("connetion Accepted\n");
    if(iRet<0) {perror("error on connect on unix socket ");exit(-1);}
    live(sMyName,mySockFd);

}
void initNet(char* sMyName, char* sIpAdress,short iPort)
{
    printf("Activating client with name '%s' and ip: %s and port: %d\n",sMyName,sIpAdress,iPort);
    struct in_addr targetAddres;
    if(inet_aton(sIpAdress,&targetAddres)==0) {printf("fail on getting ip Addres from string literal(%s)\n",sIpAdress); exit(-1);}
    if(iPort<1024)
    {
        printf("specify port ABOVE 1024\n");
        exit(-1);
    }
    netSockStruct.sin_family=AF_INET;
    netSockStruct.sin_port=htons(iPort);
    netSockStruct.sin_addr=targetAddres;
    int mySockFd=socket(AF_INET,SOCK_DGRAM,0);
    if(mySockFd<0) {perror("error on creating my socket(net) "); exit(-1);}
    if(connect(mySockFd,(struct sockaddr*)&netSockStruct,sizeof(struct sockaddr_in))<0) {perror("error on connect on net socket ");exit(-1);}

    live(sMyName,mySockFd);
}


void live(char* sMyName, int mySockFd)
{
    message msg;
    msg.m_iClientId=0;
    msg.m_length=strlen(sMyName);
    msg.m_operationId=0;
    msg.m_type=I_LIVE;
    strcpy(msg.m_message,sMyName);
    int iRet;
    printf("trying to send welcom message\n");
    iRet=write(mySockFd,&msg,sizeof(message));
    if(iRet<0) {perror("fail on sending welcome message, exting ");exit(-1);}
    printf("trying to read ACCepred/Denied message\n");
    iRet=read(mySockFd,&msg,sizeof(message));
    printf("succesfully read welcomeMessage\n");
    if(iRet<0) {perror("fail on reading accept/denied, exting ");exit(-1);}
    if(msg.m_type==DECLINED)
    {
        printf("i was declined :(  :( :( \n");
        if(close(mySockFd)<0) {perror("error on close");}
        exit(-1);
    }
    printf("i was accepted ^^\n");
    while (iCanLive)
    {
        iRet=read(mySockFd,&msg,sizeof(message));
        if(msg.m_type!=OPERATOR_ADD &&msg.m_type!=OPERATOR_DIV &&msg.m_type!=OPERATOR_MUL &&msg.m_type!=OPERATOR_SUB &&msg.m_type!=I_LIVE &&msg.m_type!=POKE &&msg.m_type!=ACCEPTED &&msg.m_type!=DECLINED &&msg.m_type!=ANSWER &&msg.m_type!=LOGOUT)
        {
            #ifdef DEBUGPRINT
            printf("got unexcpected message of type:%d, ignoring...\n",msg.m_type);
            continue;
            #endif // DEBUGPRINT
        }
        if(iRet<0)
            {
                //if(errno==EINTR) break;
                perror("fail on reading causal message, exting ");
                exit(-1);
            }
        if(msg.m_type==POKE)
        {
            iRet=write(mySockFd,&msg,sizeof(message)); // re send message
            if(iRet<0) {perror("fail on sending welcome message, exting ");exit(-1);}
            continue;
        }

        if(msg.m_type!=OPERATOR_ADD && msg.m_type!=OPERATOR_SUB && msg.m_type!=OPERATOR_MUL && msg.m_type!=OPERATOR_DIV )
        {
            #ifdef DEBUGPRINT
            printf("unexcpected message type :%d\n",msg.m_type);
            #endif // DEBUGPRINT
            continue;
        }
        if(msg.m_length!=sizeof(double)*2)
        {
            #ifdef DEBUGPRINT
            printf("unexcpcted messega with operator type(%d), and invalid length %d  (should be %d)\n",msg.m_type,msg.m_length,sizeof(double)*2);
            #endif // DEBUGPRINT
            continue;
        }

        double args[2];
        double* dP=(double*) msg.m_message;
        args[0]=*dP;
        args[1]=*(dP+1);
        double out;
        switch(msg.m_type)
        {
        case OPERATOR_ADD:
            out=args[1]+args[0];
            break;
        case OPERATOR_SUB:
            out=args[0]-args[1];
            break;
        case OPERATOR_MUL:
            out=args[0]*args[1];
            break;
        case OPERATOR_DIV:
            if(args[1]==0) {out=0;break;}
            out=args[0]/args[1];
            break;
        default:
            printf("in default statment, where it cannot be\n");
        }
        *((double *)msg.m_message)=out;
        #ifdef DEBUGPRINT_
        printf("tried to inser out into message with outcome %f\n",msg.m_message);
        #endif // DEBUGPRINT
        msg.m_length=sizeof(double);
        msg.m_type=ANSWER;

        iRet=write(mySockFd,&msg,sizeof(message)); // re send message
        if(iRet<0) {perror("fail on sending ANSWER message, exting ");exit(-1);}
    }

    clearMessage(&msg);
    msg.m_type=LOGOUT;
    int iErrors=0;
    if(write(mySockFd,&msg,sizeof(message))<0) {perror("fail on sending logout");iErrors++;}
    if(close(mySockFd)<0) {perror("error on close");iErrors++;}
    if(iErrors==0)
    {
        printf("logged out succesfuly\n");
    }

}







/*
./client name local /tmp/hue.sock
./client name net 192.0.0.0 3876
*/

void printParseError(const char* errmsg)
{
    printf("[%s]Error on run info should look like\n./client name local /tmp/hue.sock\nor\n./client name net 127.0.0.1 3876\n",errmsg);
    exit(-1);
}


int main(int args, char* argv[])
{
    printf("DTGRAM VERSION ");
    byte type;
    signal(SIGINT,signalHldr);
    if(args==4)
    {
        type=UNIX_MODE;
        if(argv[2][0]!='l')
        {
            printParseError("3 arguments given, yet type not local");
        }
        if(strlen(argv[3])>=MAX_UNIX_SOCKET_NAME_LEN-1)
        {
            printParseError("unix socket name too long");
        }

    }
    else if(args==5)
    {
        type=NET_MODE;
        if(argv[2][0]!='n')
        {
            printParseError("4 arguments given, yet type not net");
        }

    }
    else
    {
        printParseError("invalid number of arguments");
    }



    if(strlen(argv[1])>=MAX_CLIENT_NAME_LEN-1)
    {
        printParseError("Client name too long");
    }

    if(type==NET_MODE)
    {
        initNet(argv[1],argv[3],atoi(argv[4]));
    }
    if(type==UNIX_MODE)
    {
        initUnix(argv[1],argv[3]);
    }



    printf("Hello world!\n");
    return 0;
}
