#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

/*
Sig1 is SIGUSR1 or chosen SIGRT
sig2 is SIGUSR2 or chosen SIGRT


*/



#define true 1
#define false 0

int mode=3;
int L=100;
char fMother=true;
char fConf=true;
char fReadyToDie=false;

int iMotherPid;
int iChildPid=0;
int signalRecievedMother=0;
int signalSendMother=0;
int signalRecievedChild=0;

void hndlSigInit(int sig)
{
    if(!fMother) return;

    if(iChildPid !=0)
    {
        kill(9,iChildPid);
        int a;
        wait(&a);
    }

    exit(-1);
}

void hndlSig1(int sig)
{
    int myPId=getpid();

    if(fMother)
    {
        printf("signal Recieved Mother\n");
        signalRecievedMother++;
        fConf=true;
    }
    else
    {
        printf("signal Recieved Child\n");
        signalRecievedChild++;
        if( (mode==1 || mode ==2))
        {
            printf("child send\n");
            kill(getppid(),SIGUSR1);
        }
        if(mode==3)
        {
            printf("child send\n");
            kill(getppid(),SIGRTMIN+0);
        }

    }
}

void hndlSig2(int sig)
{
    if(!fMother)
        fReadyToDie=true;
}


void childLive()
{
    fMother=false;
    printf("\n\n\nchild lives\n\n");
    fMother=false;
    fConf=false;

    sigset_t toBlockSet;
    sigfillset(&toBlockSet);

    if(mode == 1 || mode == 2)
    {
        sigdelset(&toBlockSet,SIGUSR1);
        sigdelset(&toBlockSet,SIGUSR2);
    }
    else
    {
        sigdelset(&toBlockSet,SIGRTMIN+0);
        sigdelset(&toBlockSet,SIGRTMIN+1);
    }
    sigprocmask(SIG_SETMASK,&toBlockSet,NULL);
    while(!fReadyToDie)
    {
        //printf("child Loop: fReadyToDie: %d ; fConf : %d \n",fReadyToDie,fConf);
        sleep(0);
    }
    printf("dziecko otrzymalo %d sygnalow\n",signalRecievedChild);
    exit(0);
}


int main(int args,char* argv[])
{
    if(args==3)
    {
        mode=atoi(argv[2]);
        L=atoi(argv[1]);
    }

    struct sigaction act1;
    act1.sa_flags=0;
    sigemptyset(&act1.sa_mask);
    act1.sa_handler=hndlSig1;
    sigaction(SIGUSR1,&act1,NULL);
    sigaction(SIGRTMIN+0,&act1,NULL);


    struct sigaction act2;
    act2.sa_flags=0;
    sigemptyset(&act2.sa_mask);
    act2.sa_handler=hndlSig2;
    sigaction(SIGUSR2,&act2,NULL);
    sigaction(SIGRTMIN+1,&act2,NULL);

    struct sigaction act3;
    act3.sa_flags=0;
    sigemptyset(&act3.sa_mask);
    act3.sa_handler=hndlSigInit;
    sigaction(SIGINT,&act3,NULL);






    int childPid=fork();
    if(childPid==0)
    {
        fMother=false;
        childLive();
    }

    sleep(1);
    sleep(1);

    printf("\n\n\n mother lives \n\n\n");
    sleep(1);
    int myPid=getpid();
    iChildPid=childPid;
    iMotherPid=getpid();
    for(int i=0;i<L;)
    {
        sleep(0);
        if(mode==1)
        {
            kill(childPid,SIGUSR1);
            signalSendMother++;
            i++;
        }
        if(mode==2 && fConf==true)
        {
            kill(childPid,SIGUSR1);
            signalSendMother++;
            fConf=false;
            i++;
        }
        if(mode==3)
        {
            kill(childPid,SIGRTMIN+0);
            signalSendMother++;
            i++;
        }
        //sleep(1);
        //printf("alive");
    }

    printf("%d signals send\n",L);
    if(mode==1)
        {
            kill(childPid,SIGUSR2);

        }
    if(mode==2 && fConf==true)
        {
            kill(childPid,SIGUSR2);
            fConf=false;
        }
    if(mode==3)
        {
            kill(childPid,SIGRTMIN+1);
        }
    int stat;
    printf("\n\nmother awaits \n\n");
    wait( &stat);
    printf("sygnaly wysalne przez Matke %d\n",signalSendMother);
    printf("sygnaly odebrane przez Matke %d\n",signalRecievedMother);
    return 0;
}
