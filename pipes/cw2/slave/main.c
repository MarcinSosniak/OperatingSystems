#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#define true 1
#define false 0


char live(char* path, int n)
{
    srand(time(NULL));
    printf("path: %s\n",path);
    char chPID[10];
    sprintf(chPID,"%d",getpid());
    printf("%s\n",chPID);
    int fd=open(path,O_WRONLY);
    if(fd<0)
    {
        printf("%d\n",errno);
        return false;
    }
    FILE* fhndl;
    char chBuff[100];
    char chOut[125];
    int iBytesRead;
    int iSleepT;
    int hue;
    for(int i=0;i<n;i++)
    {
        fhndl=popen("date","r");
        if(fhndl==NULL) return false;
        iBytesRead=fread(chBuff,1,50,fhndl);
        for(int k=0;k<100;k++)
        {
            if(chBuff[k]=='\n')
            {
                chBuff[k]='\0';
            }
        }
        sprintf(chOut,"%s>%s\n",chPID,chBuff);
        printf("%s",chOut);
        hue=write(fd,chOut,strlen(chOut));
        printf("printed %d characters | working... \n",hue);
        fclose(fhndl);
        iSleepT=(rand()%3+1);
        sleep(iSleepT);
    }
    close(fd);
    return true;
}

int main(int args,char* argv[])
{


    int n=10;
    char pathBuff[]="/tmp/potok";
    char* path=pathBuff;
    if(args==3)
    {
        path=argv[1];
        n=atoi(argv[2]);
    }
    if(!live(path,n))
        printf("a fatal error has occured\n");

    //printf("Hello world!\n");

    return 0;
}
