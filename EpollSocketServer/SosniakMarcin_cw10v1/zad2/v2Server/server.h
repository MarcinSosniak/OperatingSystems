#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include "data_structs_defs.h"
#include "que.h"


#include <pthread.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>



#define MAX_CLIENTS 20
#define DEFAULT_MAX_RAND 4*MAX_CLIENTS // has to be more than 0
#define TIME_DIFF_TO_KICK 10.0  // time in seconds as double
#define TIME_FOR_THREAD_INIT 3


#define THREAD_STATUS_NOT_READY 0
#define THREAD_STATUS_READY 1
#define THREAD_STATUS_TO_CLEANUP 2
#define THREAD_STATUS_CLEANED_UP 3



#define DEBUGPRNT 1
#define _GNU_SOURCE


extern struct sockaddr_un unixSockStruct; // name inicialized in main();
extern struct sockaddr_in netSockStruct;
extern volatile byte communicationThreadStatus;
extern volatile byte consoleReadThreadStatus;
extern volatile byte pokingThreadStatus;

#define UNIX_MODE 0
#define NET_MODE 1



typedef struct
{
    char m_sName[MAX_CLIENT_NAME_LEN+1];
    int m_iSocketFd; // not used
    time_t m_LastActiveTime;
    struct sockaddr* m_sockInfo;
    socklen_t m_sockaddr_len;
    byte mode;
}clientInfo;



/*
some struct from here moved to serve_structs.h


*/

typedef struct
{
    time_t m_addedTime;
    int m_socketFd;
}tmpClient;


//init
int generalServerInit(char* sName,uint16_t iPortNumber);
//

void* CommunicationThread(void*);
void* PokingThread(void*);
void* ConsoleReadThread(void*);

void  killSocket(int fd,bool fVerbose);
int InitClientTable();
bool fThreadsDone();
int sendAdd(int iClientId, double * out); // if iClient id <0 it will take random avaible client

#endif // SERVER_STRUCT_H_INCLUDED
