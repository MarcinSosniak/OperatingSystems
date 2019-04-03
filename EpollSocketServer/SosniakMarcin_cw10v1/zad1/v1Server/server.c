#include "server.h"


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


#define EPOLL_DEBUG 1

/*
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||

*/

clientInfo clientsTab[MAX_CLIENTS];
bool fClientTableInit=false;
messageQue mQ;
taskQue tQ;


struct sockaddr_un unixSockStruct; // name inicialized in main();
struct sockaddr_in netSockStruct;

volatile byte communicationThreadStatus=THREAD_STATUS_NOT_READY;
volatile byte consoleReadThreadStatus=THREAD_STATUS_NOT_READY;
volatile byte pokingThreadStatus=THREAD_STATUS_NOT_READY;

pthread_mutex_t clientTabMutexReal=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t* clientTabMutex=&clientTabMutexReal;


int fdConnEpoll;


void mutexLock(pthread_mutex_t *m)
{
    int iRet=pthread_mutex_lock(m);
    if(iRet!=0) {printf("fail on mutex grab with error:%d\n",iRet); exit(-1);}
}
void mutexUnLock(pthread_mutex_t *m)
{
    int iRet=pthread_mutex_unlock(m);
    if(iRet!=0) {printf("fail on mutex release with error:%d\n",iRet); exit(-1);}
}

int returnAndUnlock(pthread_mutex_t *m,int val)
{
    mutexUnLock(m);
    return val;
}


/*
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
            Init()
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
*/
int generalServerInit(char* sName,uint16_t iPortNumber) // port number still as normal int
{
    msQueInit(&mQ);
    tQueInit(&tQ);
    InitClientTable();
    unixSockStruct.sun_family=AF_UNIX;
    strcpy(unixSockStruct.sun_path,sName);
    netSockStruct.sin_family=AF_INET;
    struct in_addr addr;
    addr.s_addr=INADDR_ANY;
    netSockStruct.sin_addr=addr;
    // netSockStruct port should be inicialized in main()
    netSockStruct.sin_port=htons(iPortNumber);
    // rember htons!!!!!!! ;
    pthread_t ptRet;
    int iRet;
    time_t creationTime;
    time_t currentTime;
    currentTime=time(&creationTime);
    iRet=pthread_create(&ptRet,NULL,CommunicationThread,NULL);
    if(iRet<0) {perror("fail on creating communications thread, exitting"); exit(-1);}
    while(communicationThreadStatus!=THREAD_STATUS_READY)
    {
        time(&currentTime);
        if(difftime(currentTime,creationTime)>TIME_FOR_THREAD_INIT)
        {
            printf("fail on waiting for creation Thread, exitting\n");
            exit(-1);
        }
    }

    currentTime=time(&creationTime);
    iRet=pthread_create(&ptRet,NULL,ConsoleReadThread,NULL);
    if(iRet<0) {perror("fail on creating ConsoleReadThread , exitting"); exit(-1);}
    while(consoleReadThreadStatus!=THREAD_STATUS_READY)
    {
        time(&currentTime);
        if(difftime(currentTime,creationTime)>TIME_FOR_THREAD_INIT)
        {
            printf("fail on waiting for ConsoleReadThread , exitting\n");
            exit(-1);
        }
    }

    currentTime=time(&creationTime);
    iRet=pthread_create(&ptRet,NULL,PokingThread,NULL);
    if(iRet<0) {perror("fail on creating PokingThread , exitting"); exit(-1);}
    while(pokingThreadStatus!=THREAD_STATUS_READY)
    {
        time(&currentTime);
        if(difftime(currentTime,creationTime)>TIME_FOR_THREAD_INIT)
        {
            printf("fail on waiting for PokingThread , exitting\n");
            exit(-1);
        }
    }


    return 0;
}



int InitClientTable()
{
    if(fClientTableInit) return -EAGAIN;
    int i,k;
    for(i=0;i<MAX_CLIENTS;i++)
    {
        clientsTab[i].m_iSocketFd=-1;
        for(k=0;k<MAX_CLIENT_NAME_LEN+1;k++)
        {
            clientsTab[i].m_sName[k]='\0';
        }
    }
    srand(time(NULL));
    fClientTableInit=true;
    return 0;
}



/*
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
            CLIENT TABLE MANIPULATION WITH MUTEX LOCK ON !
            DO NOT USE THOSE FUNCTIONS WITHOUT LOCK
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
*/
bool canAddClient( char* sClientName)
{
    int i=0;
    bool bFoundSlot=false;
    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(clientsTab[i].m_iSocketFd==-1)
        {
            bFoundSlot=true;
            continue;
        }
    }
    if(!bFoundSlot) return false;
    char* sClientNameCpy;
    if(strlen(sClientName)>MAX_CLIENT_NAME_LEN)
    {
        char sClientNameCpyBuffor[strlen(sClientName)+1];
        strcpy(sClientNameCpyBuffor,sClientName);
        sClientNameCpyBuffor[MAX_CLIENT_NAME_LEN]='\0';
        sClientNameCpy=sClientNameCpyBuffor;
    }
    else
    {
        sClientNameCpy=sClientName;// it's not too long
    }

    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(strcmp(clientsTab[i].m_sName,sClientNameCpy)==0)
        {
            return false;
        }
    }
    return true;
}

int addClientToTable(char* sClientName, int socketFd)
{
    int i;
    char* sClientNameCpy;
    if(strlen(sClientName)>MAX_CLIENT_NAME_LEN)
    {
        char sClientNameCpyBuffor[strlen(sClientName)+1];
        strcpy(sClientNameCpyBuffor,sClientName);
        sClientNameCpyBuffor[MAX_CLIENT_NAME_LEN]='\0';
        sClientNameCpy=sClientNameCpyBuffor;
    }
    else
    {
        sClientNameCpy=sClientName;// it's not too long
    }

    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(strcmp(clientsTab[i].m_sName,sClientNameCpy)==0)
        {
            return -EINVAL;
        }
    }

    bool fFound=false;
    for(i=0;i<MAX_CLIENTS && !fFound;i++)
    {
        if(clientsTab[i].m_iSocketFd==-1)
        {
            fFound=true;
            clientsTab[i].m_iSocketFd=socketFd;
            strcpy(clientsTab[i].m_sName,sClientNameCpy);
            time(&clientsTab[i].m_LastActiveTime);
            break;
        }
    }
    if(fFound) return 0;
    return -EAGAIN;
}




bool fAnyClientWithLock()
{
    int i=0;
    for(;i<MAX_CLIENTS;i++)
    {
        if(clientsTab[i].m_iSocketFd!=-1 && clientsTab[i].m_sName[0]!='\0') return true;
    }
    return false;
}


int checkClientWithLock(int iClientNr)
{
    if(iClientNr<0 || iClientNr>=MAX_CLIENTS) return -EINVAL;
    if(clientsTab[iClientNr].m_iSocketFd==-1) return -ENXIO;
    return 0;

}

int removeClientFromTable(int iClientNr)
{
    if(iClientNr<0 || iClientNr >= MAX_CLIENTS) return -EINVAL;
    if(clientsTab[iClientNr].m_iSocketFd==-1) return -EAGAIN;
    clientsTab[iClientNr].m_iSocketFd=-1;
    clientsTab[iClientNr].m_LastActiveTime=0;
    clientsTab[iClientNr].m_sName[0]='\0';
    return 0;
}

int addAnonymousClientToTable(int socketFd)
{
    int i;
    if(socketFd<0) return -EINVAL;
    for(i=0;i<MAX_CLIENTS ;i++)
    {
        if(clientsTab[i].m_iSocketFd==-1)
        {

            clientsTab[i].m_iSocketFd=socketFd;
            clientsTab[i].m_sName[0]='\0';
            time(&clientsTab[i].m_LastActiveTime);
            #ifdef DEBUGPRNT
            printf("\nAdded anonyous client [%d]\n",i);
            #endif // DEBUGPRNT
            return 0;
        }
    }
    return -EAGAIN;
}


int removeClientWithLock(int iClientId)
{
    if(checkClientWithLock(iClientId)!=0) return iClientId;
    if(epoll_ctl(fdConnEpoll,EPOLL_CTL_DEL,clientsTab[iClientId].m_iSocketFd,NULL)<0) {perror("failure on removing client from epoll");}
    if(shutdown(clientsTab[iClientId].m_iSocketFd,SHUT_RDWR)!=0) perror("fail on shutdown on inpossible to add Client");
    if(close(clientsTab[iClientId].m_iSocketFd)!=0) perror("failure on close on impossible to add Client");
    return removeClientFromTable(iClientId);
}

int findClientBySocketWithLock(socketFd)
{
    int i=0;
    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(clientsTab[i].m_iSocketFd==socketFd)
            return i;
    }
    return -1;
}




/*
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
            CONTROL CLIENTS OPEN FUNCTIONS
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
*/

int addClient( char* name,tmpClient client)
{
    mutexLock(clientTabMutex);
    if(!canAddClient(name)) return returnAndUnlock(clientTabMutex, -EINVAL);
    int iRet=addClientToTable(name,client.m_socketFd);
    if(iRet>=0) return returnAndUnlock(clientTabMutex, iRet);
    // else ///
    if(shutdown(client.m_socketFd,SHUT_RDWR)!=0) perror("fail on shutdown on inpossible to add Client");
    if(close(client.m_socketFd)!=0) perror("failure on close on impossible to add Client");
    return returnAndUnlock(clientTabMutex, iRet);
}
int addAnonymousClient(int iSockFd)
{
    mutexLock(clientTabMutex);
    int iRet=addAnonymousClientToTable(iSockFd);
    if(iRet==0) return returnAndUnlock(clientTabMutex, 0);
    if(shutdown(iSockFd,SHUT_RDWR)!=0) perror("fail on shutdown on impossible to add Anonymous Client");
    if(close(iSockFd)!=0) perror("failure on close on impossible to add Anonymous Client");
    return returnAndUnlock(clientTabMutex,iRet);
}


int removeClient(int iClientId)
{
    mutexLock(clientTabMutex);
    int out=removeClientWithLock(iClientId);
    return returnAndUnlock(clientTabMutex,out);
}


int checkClient(int iClientNr)
{
    mutexLock(clientTabMutex);
    if(iClientNr<0 || iClientNr>=MAX_CLIENTS) return returnAndUnlock(clientTabMutex, -EINVAL);
    if(clientsTab[iClientNr].m_iSocketFd==-1) return returnAndUnlock(clientTabMutex, -ENXIO);
    return returnAndUnlock(clientTabMutex, 0);
}

int getRandomClient(int iMaxRand) // return either Index >= 0, or estandard ERRNO error code as negative
{
    mutexLock(clientTabMutex);
    if(!fAnyClientWithLock()) return returnAndUnlock(clientTabMutex, -EAGAIN); // meaning resorce not avaible, lack of clients
    if(iMaxRand<=0) iMaxRand=DEFAULT_MAX_RAND;
    int iRandVal=rand()%iMaxRand+1;
    int iCounterToRandVal=0;
    int i=-1;
    while(iCounterToRandVal<iRandVal)
    {
        if(!fAnyClientWithLock()) return returnAndUnlock(clientTabMutex, -EAGAIN); // just to make sure that clients won't be killed;
        i++;
        if(i>=MAX_CLIENTS) i=0;
        if(clientsTab[i].m_iSocketFd!=-1 && clientsTab[i].m_sName[0]!='\0') iCounterToRandVal++;
    }
    return returnAndUnlock(clientTabMutex, i);
}

int removeClientBySocket(int socketFd)
{
    mutexLock(clientTabMutex);
    int id;
    id=findClientBySocketWithLock(socketFd);
    if(id < 0) return returnAndUnlock(clientTabMutex, -EINVAL);
     return returnAndUnlock(clientTabMutex,removeClientWithLock(id));
}

int findClientBySocket(int socketFd)
{
    mutexLock(clientTabMutex);
    int out=findClientBySocketWithLock(socketFd);
    return returnAndUnlock(clientTabMutex,out);
}

bool fAnyClient()
{
    mutexLock(clientTabMutex);
    bool out=fAnyClientWithLock();
    mutexUnLock(clientTabMutex);
    return out;
}

int renameClientBySocket(char* name,int socketFd)
{
    mutexLock(clientTabMutex);
    int i;
    if (socketFd<0) return returnAndUnlock(clientTabMutex,-EINVAL);
    if (name[0]=='\0') return returnAndUnlock(clientTabMutex,-EINVAL);
    char* sClientNameCpy;
    if(strlen(name)>MAX_CLIENT_NAME_LEN)
    {
        char sClientNameCpyBuffor[strlen(name)+1];
        strcpy(sClientNameCpyBuffor,name);
        sClientNameCpyBuffor[MAX_CLIENT_NAME_LEN-1]='\0';
        sClientNameCpy=sClientNameCpyBuffor;
    }
    else
    {
        sClientNameCpy=name;// it's not too long
    }

    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(strcmp(name,clientsTab[i].m_sName)==0)
        {
            return returnAndUnlock(clientTabMutex,-EEXIST);
        }
    }

    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(clientsTab[i].m_iSocketFd==socketFd)
        {
            strcpy(clientsTab[i].m_sName,sClientNameCpy);
            time(&clientsTab[i].m_LastActiveTime);
            return returnAndUnlock(clientTabMutex, 0);
        }
    }
    return returnAndUnlock(clientTabMutex,-EAGAIN);
}


/*
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
            SEND FUNCTIONS
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
*/

int sendAccepted(int iClientNr)
{
    if(checkClient(iClientNr)!=0) return checkClient(iClientNr);
    message msg;
    msg.m_length=0;
    msg.m_message[0]='\0';
    msg.m_iClientId=iClientNr;
    msg.m_type=ACCEPTED;
    if(write(clientsTab[iClientNr].m_iSocketFd,&msg,sizeof(message))<0) {printf("sock fd= %d\n",clientsTab[iClientNr].m_iSocketFd);perror("error on write send Accepted"); return errno;}
    //insert sendmessage here;
    return 0;
}

int sendDenied(int iClientNr)
{
    if(checkClient(iClientNr)<0) return checkClient(iClientNr);
    message msg;
    msg.m_length=0;
    msg.m_message[0]='\0';
    msg.m_iClientId=iClientNr;
    msg.m_type=DECLINED;
    if(write(clientsTab[iClientNr].m_iSocketFd,&msg,sizeof(message))<0) {printf("sock fd= %d\n",clientsTab[iClientNr].m_iSocketFd);perror("error on write send Dienied"); return errno;}
    return 0;
}

int sendOP(int iClientNr,byte operatorNr,double arg1, double arg2,int operationId)
{
    if(checkClient(iClientNr)!=0) return checkClient(iClientNr);
    if(operatorNr!=OPERATOR_ADD && operatorNr!=OPERATOR_SUB && operatorNr!=OPERATOR_DIV && operatorNr!=OPERATOR_MUL ) return -EINVAL;
    message msg;
    msg.m_length=sizeof(double)*2;
    msg.m_type=operatorNr;
    msg.m_iClientId=iClientNr;
    double* tmpP=(double*)msg.m_message;
    tmpP[0]=arg1;
    tmpP[1]=arg2;
    msg.m_operationId=operationId;
    if(write(clientsTab[iClientNr].m_iSocketFd,&msg,sizeof(message))<0) {printf("sock fd= %d\n",clientsTab[iClientNr].m_iSocketFd);perror("error on write sendOp"); return errno;}
    //insertSendmssg;
    return 0;
}

int sendPoke(int iClientNr)
{
    if(checkClient(iClientNr)!=0) return checkClient(iClientNr);
    message msg;
    msg.m_length=0;
    msg.m_message[0]=0;
    msg.m_iClientId=iClientNr;
    msg.m_type=POKE;
    if(write(clientsTab[iClientNr].m_iSocketFd,&msg,sizeof(message))<0)
    {
        if(errno==EPIPE)
        {
            #ifdef DEBUGPRNT
            perror("pipe interrupted by client");
            #endif // DEBUGPRNT
            return errno;
        }
        printf("sock fd= %d\n",clientsTab[iClientNr].m_iSocketFd);
        perror("error on write Poke");
        return errno;
    }
    return 0;
}

/*
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
            THREAD FUNCTIONS
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
*/

///////////////////// POKE/////////////////////////////////////////////////////

void* PokingThread(void* t)
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    int i;
    time_t currentTime;
    int iRet;
    pokingThreadStatus=THREAD_STATUS_READY;
    while( pokingThreadStatus!=THREAD_STATUS_TO_CLEANUP)
    {
        time(&currentTime);
        for(i=0;i<MAX_CLIENTS;i++)
        {
            if(clientsTab[i].m_iSocketFd==-1) continue;
            if(difftime(currentTime, clientsTab[i].m_LastActiveTime) > TIME_DIFF_TO_KICK && clientsTab[i].m_LastActiveTime!=0)
            {
                removeClient(i);
                continue;
            }
            if(clientsTab[i].m_sName[0]!='\0') {iRet=sendPoke(i); continue;} // don't poke unfully register clients
            if(iRet==EPIPE)
            {
                removeClient(i);
                continue;
            }

            if(iRet!=0)
            {
                clientsTab[i].m_LastActiveTime=0;// let's allow him life if it's our mistake;
                #ifdef DEBUGPRNT
                printf("\nFailure on poking client (%d) with error:%d \n",i,iRet);
                #endif // DEBUGPRNT
            }
        }
        if(!fAnyClient())
            #ifdef DEBUGPRNT
            printf("No Clients connected\n");
            #endif // DEBUGPRNT

        sleep(3);
    }

    pokingThreadStatus=THREAD_STATUS_CLEANED_UP;
    return NULL;
}

//////////////////////COMMUNICATION////////////////////////////////////////////

int initUnixSocket()// return socket fd, or Error code
{
    int fdUnixSock;
    fdUnixSock=socket(AF_UNIX,SOCK_STREAM | SOCK_NONBLOCK,0);
    if(fdUnixSock == -1)
    {
        perror("Failure on openning unix socket, exitting\n");
        exit(-1);
    }
    int iRet;
    iRet=bind(fdUnixSock,(struct sockaddr*)&unixSockStruct,sizeof(struct sockaddr_un));
    if(iRet<0)
    {
        perror("Failure on binding unix socket, exitting\n");
        exit(-1);
    }
    iRet=listen(fdUnixSock,MAX_CLIENTS);
    if(iRet<0)
    {
        perror("Failure on listening to unix socket, exitting\n");
        exit(-1);
    }
    return fdUnixSock;
}

int initNETSocket()
{
    int fdNetSock;
    fdNetSock=socket(AF_INET,SOCK_STREAM | SOCK_NONBLOCK,0);
    if(fdNetSock == -1)
    {
        perror("Failure on openning net socket, exitting\n");
        exit(-1);
    }
    int iRet;
    iRet=bind(fdNetSock,(struct sockaddr*)&netSockStruct,sizeof(struct sockaddr_in));
    if(iRet<0)
    {
        perror("Failure on binding net socket, exitting\n");
        exit(-1);
    }
    iRet=listen(fdNetSock,MAX_CLIENTS);
    if(iRet<0)
    {
        perror("Failure on listening to net socket, exitting\n");
        exit(-1);
    }
    struct sockaddr_in netDefines;
    socklen_t socksize=sizeof(struct sockaddr_in);
    if(getsockname(fdNetSock,&netDefines,&socksize)<0) {perror("fail on getting back IP, exitting"); printf("%d\n",errno);exit(-1);}
    printf("Server IP adress: '%s' \n",inet_ntoa((netDefines.sin_addr)));
    return fdNetSock;
}


int initListenEpoll(int fdUnix,int fdNet)
{
    int fdListenEpoll=epoll_create1(0);
    if(fdListenEpoll<0) {perror("failure on creating epoll"); exit(-1);}

    struct epoll_event evUnix;
    evUnix.data.fd=fdUnix;
    evUnix.events=EPOLLIN ; // add EPOLLET  ?
    if(epoll_ctl(fdListenEpoll,EPOLL_CTL_ADD,fdUnix,&evUnix)){perror("error on adding unix Listen socket to epoll"); exit(-1);}

    struct epoll_event evNet;
    evNet.data.fd=fdNet;
    evNet.events=EPOLLIN ; // add EPOLLET  ?
    if(epoll_ctl(fdListenEpoll,EPOLL_CTL_ADD,fdNet,&evNet)){perror("error on adding net Listen socket to epoll"); exit(-1);}

    return fdListenEpoll;
}
int addToEpoll(int epollFD,int socketFd)
{
    struct epoll_event ev;
    ev.data.fd=socketFd;
    ev.events=EPOLLIN;
    if(epoll_ctl(epollFD,EPOLL_CTL_ADD,socketFd,&ev)) {perror("errror on adding anonmous client to epoll");}
    #ifdef EPOLL_DEBUG
    printf("added To epoll with socket:%d\n",socketFd);
    #endif // EPOLL_DEBUG

    return 0;

}





void* CommunicationThread(void * commonSpace) // it will also make asynchronous print out;
{
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,NULL);
    int fdUnixSock=initUnixSocket();
    int fdNetSock=initNETSocket();
    int fdListenEpoll=initListenEpoll(fdUnixSock,fdNetSock);
    struct epoll_event listenEventsTab[2];
    struct epoll_event connEventsTab[MAX_CLIENTS];
    fdConnEpoll=epoll_create1(0);
    if (fdConnEpoll<0) {perror("Fail on creating eppol for client connectios, exiting \n");exit(-1);}
    int i;
    int iTmpSockFd;
    int iRet;
    int iEpollListenRet;
    int iEpollClientsRet;
    message tmpMSG;
    int iTmpClientId;


    communicationThreadStatus=THREAD_STATUS_READY;
    while(communicationThreadStatus!=THREAD_STATUS_TO_CLEANUP)
    {

        // check listening sockets
        iEpollListenRet=epoll_wait(fdListenEpoll,listenEventsTab,2,1);
        if(iEpollListenRet < 0) perror("error on epoll_wait on iEpollListenRet");
        if(iEpollListenRet > 0)
        {
            for(i=0;i<iEpollListenRet;i++)
            {
                iTmpSockFd=accept4(listenEventsTab[i].data.fd,NULL,NULL,SOCK_NONBLOCK);
                if(iTmpSockFd<0) perror("error on accept");
                else
                {
                    iRet=addAnonymousClient(iTmpSockFd);
                    if(iRet<0) perror("error on adding anonymous client");
                }
                if(iRet>=0)
                {
                    addToEpoll(fdConnEpoll,iTmpSockFd);
                }
            }
        }
        ///////////////////////////////////////////////////////////////////////
        //check client sockets
        iEpollClientsRet=epoll_wait(fdConnEpoll,connEventsTab,MAX_CLIENTS,1);
        if(iEpollClientsRet < 0) perror("error on epoll_wait on iEpollClientsRet");
        if(iEpollClientsRet > 0)
        {
            for(i=0;i<iEpollClientsRet;i++)
            {
                clearMessage(&tmpMSG);
                if(read(connEventsTab[i].data.fd,&tmpMSG,sizeof(tmpMSG))<0) {perror("fail on reading");continue;}
                switch(tmpMSG.m_type)
                {
                case EMPTY_MESSAGE:
                    #ifdef DEBUGPRNT
                    printf("got empty meesage from client socket id %d\n",tmpMSG.m_iClientId);
                    exit(-1);
                    #endif // DEBUGPRNT
                    break;
                case POKE:
                    time(&(clientsTab[tmpMSG.m_iClientId].m_LastActiveTime));
                    break;
                case I_LIVE:
                    iRet=renameClientBySocket(tmpMSG.m_message,connEventsTab[i].data.fd);
                    if(iRet<0 && iRet!= -EAGAIN) printf("fail on updating name, socket:%d; name '%s'",connEventsTab[i].data.fd,(char *)tmpMSG.m_message);
                    if(iRet==-EEXIST)
                    {
                        #ifdef DEBUGPRNT
                        printf("\n found with the same name \n\n");
                        #endif // DEBUGPRNT
                        iTmpClientId=findClientBySocket(connEventsTab[i].data.fd);
                        if(iTmpClientId<0) break;
                        else
                        {
                            sendDenied(iTmpClientId);
                        }
                    }
                    if(iRet==-EAGAIN)
                    {
                        sendDenied(iTmpClientId);
                    }
                    if(iRet==0)
                    {
                        iTmpClientId=findClientBySocket(connEventsTab[i].data.fd);
                        if(iTmpClientId<0) break;
                        else
                        {
                            sendAccepted(iTmpClientId);
                        }
                    }
                    break;

                case ANSWER:

                    printf("Answer #%d, by '%s' is... : %f\n",tmpMSG.m_operationId,clientsTab[tmpMSG.m_iClientId].m_sName,*((double*)tmpMSG.m_message));
                    time(&(clientsTab[tmpMSG.m_iClientId].m_LastActiveTime));
                    break;
                case LOGOUT:
                    iRet=removeClientBySocket(connEventsTab[i].data.fd);
                    if(iRet<0) printf("fail on LOGOUT socket=%d,ret=%d",connEventsTab[i].data.fd,iRet);
                    break;
                default:
                    printf("uknown message type :%d",tmpMSG.m_type);

                }
            }

        }
        ///////////////////////////////////////////////////////////////////////
        // check new Tasks
        if(fAnyClient() && tQueGetCount(&tQ)>0)
        {
            task newTask=tQueGet(&tQ);
            if(newTask.m_bOperation==OPERATOR_QUIT)
            {
                communicationThreadStatus=THREAD_STATUS_TO_CLEANUP;
                pokingThreadStatus=THREAD_STATUS_TO_CLEANUP;
                consoleReadThreadStatus=THREAD_STATUS_TO_CLEANUP;
                continue;
            }
            sendOP(getRandomClient(70),newTask.m_bOperation,newTask.m_dArg1,newTask.m_dArg2,newTask.m_iTaskId);
        }

    }//end while

    killSocket(fdUnixSock,true);
    killSocket(fdNetSock,true);

    for(int i=0;i<MAX_CLIENTS;i++)
    {
        if(checkClient(i)==0)
        {
            killSocket(clientsTab[i].m_iSocketFd,false);
        }
    }

    communicationThreadStatus=THREAD_STATUS_CLEANED_UP;

    return NULL;
}

//////////////////////ConsoleReadThread////////////////////////////////////////
void printParseLineError()
{
    printf("ECHO>Parsing error, invalid line\n");
}



void* ConsoleReadThread(void * commonSpace)
{
    int orderId=0;
    char line[MAX_ANSWER_LEN+1];
    int i=0;
    printf("line should look like:\n2.48 * 3.14\n operators accepted '*' '+' '-' '/'\n");
    task tk;
    consoleReadThreadStatus=THREAD_STATUS_READY;
    while(consoleReadThreadStatus!=THREAD_STATUS_TO_CLEANUP)
    {
        fgets(line,MAX_ANSWER_LEN,stdin);
        if(strstr(line,"QUIT")|| strstr(line,"quit"))
        {
            printf("quitCallAccepted\n");
            /*
            task newTask;
            newTask.m_bOperation=OPERATOR_QUIT;
            if(tQueAdd(&tQ,newTask)!=true)
            {
                printf("que temporary full");
                while(!tQueAdd(&tQ,newTask));
            }
            */
            communicationThreadStatus=THREAD_STATUS_TO_CLEANUP;
            pokingThreadStatus=THREAD_STATUS_TO_CLEANUP;
            consoleReadThreadStatus=THREAD_STATUS_TO_CLEANUP;

            continue;
        }
        orderId++;
        for(i=0;i<MAX_ANSWER_LEN && line[i]!='\0' && line[i]==' ';i++) ; // skip spaces
        if(line[i]=='\n' || i>=MAX_ANSWER_LEN) {printParseLineError();continue;}
        tk.m_dArg1=atof(line+i);
        i++;
        for(;i<MAX_ANSWER_LEN && ( (line[i]<='9' && line[i]>='0')  || line[i]=='.' || line[i]=='e' || line[i]=='E');i++);
        // /\ skipp first param, and any spaces
        for(;i<MAX_ANSWER_LEN && line[i]==' ';i++);
        if(line[i]=='\n' || i>=MAX_ANSWER_LEN) {printParseLineError();continue;}
        char op=line[i];
        switch(op)
        {
        case '+':
            tk.m_bOperation=OPERATOR_ADD;
            break;

        case '*':
            tk.m_bOperation=OPERATOR_MUL;
            break;

        case '-':
            tk.m_bOperation=OPERATOR_SUB;
            break;

        case '/':
            tk.m_bOperation=OPERATOR_DIV;
            break;
        default:
            printParseLineError();
            printf("invalid operator '%c'\n",op);
            continue;
        }
        i++;
        for(;line[i]==' ' && line[i] !='\n' && i<MAX_ANSWER_LEN;i++);//again skip all spaces
        if(line[i]=='\n' || i>=MAX_ANSWER_LEN) {printParseLineError();continue;}
        tk.m_dArg2=atof(line+i);
        tk.m_iTaskId=orderId;
        if(tQueAdd(&tQ,tk)==false)
        {
            printf("Order Que is full.  Please wait\n");
            while(tQueAdd(&tQ,tk)==false);
            printf("order Added, you may ask for more\n");
        }
        printf("ECHO: op=%c,taskid=%d,arg1=%f,arg2=%f\n",op,tk.m_iTaskId,tk.m_dArg1,tk.m_dArg2);
    }

    consoleReadThreadStatus=THREAD_STATUS_CLEANED_UP;
    return NULL;
}

/*
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
            Others
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
*/

void clearTask(task *me)
{
    me->m_bOperation=EMPTY_OPERATOR;
    me->m_dArg1=0;
    me->m_dArg2=0;
    me->m_iTaskId=-1;

}

void killSocket(int socketFd,bool fVerbose)
{
    if(shutdown(socketFd,SHUT_RDWR)<0) perror("fail shuting down socket in killSocket");
    if(close(socketFd)<0) perror("fail on closing socket");
}


bool fThreadsDone()
{
    if(communicationThreadStatus==THREAD_STATUS_CLEANED_UP)
        if(pokingThreadStatus==THREAD_STATUS_CLEANED_UP)
            if(consoleReadThreadStatus==THREAD_STATUS_CLEANED_UP)
                return true;
    return false;
}

















