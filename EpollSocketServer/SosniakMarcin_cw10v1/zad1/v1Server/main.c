#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include "data_structs_defs.h"
#include "server.h"
#include <signal.h>

#define DEFAULT_PORT_NUMBER 3876;
//#define DEFAULT_UNIX_BASE_NAME "/socketF/"


/*
    ./server1 700 /tmp/someunixsockname.sock
*/
int main(int args, char* argv[])
{
    char chNameBuff[MAX_UNIX_SOCKET_NAME_LEN];
    char default_unix_name[]="/socketF/";
    char* sUNIXname;
    uint16_t iPortNumber;
    if(args == 3 && atoi(argv[2])!=0)
    {
        sUNIXname=argv[2];
        iPortNumber=atoi(argv[1]);
    }
    else
    {

        if(args==3 && atoi(argv[2])==0)
        {
            printf("specified port cannot be 0, starting up with defaults \n");
        }
        iPortNumber=DEFAULT_PORT_NUMBER;
        sprintf(chNameBuff,"%s%d.sock",default_unix_name,getpid());
        sUNIXname=chNameBuff;
        printf("Activating with test settings\nport: %d \nUnixSockName: '%s'\n",iPortNumber,sUNIXname);
    }
    generalServerInit(sUNIXname,iPortNumber);
    while(!fThreadsDone())
    {
        ;
    }

    printf("Serwer succesfully killed\n");
    return 0;
}
