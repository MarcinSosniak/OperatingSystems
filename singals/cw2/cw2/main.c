#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>


#define true 1
#define false 0

// comment which you don't want to show
#define CREATE 1
#define ASKED 1
#define PERMISSION_SEND 1
#define REAL_TIME_SGI_REC 1
#define CHILD_DIES 1

//#define DEBUG 1

#define PAUZE 100000


struct childInf
{
    int m_PID;
    char m_fEnded;
};





char fMother=true;
int asked=0;
int askedReq=100;
struct childInf* childInfBuff;
int  chInfIndex=0;
char fPermGranted=false;


void hndlSIGINT(int sig,siginfo_t *siginfo, void* context )
{
    if(fMother)
    {
        for(int i=0;i<chInfIndex;i++)
        {
            kill(  (*(childInfBuff + chInfIndex)).m_PID,SIGKILL);
        }
        exit(-1);
    }
    else
    {
        ;// do nothing, your are going to get exterminatus soon

    }
}

void hndlSIGUSR1(int sig,siginfo_t *siginfo, void* context )
{
    if(fMother)
    {
        (*(childInfBuff+chInfIndex)).m_PID=siginfo->si_pid;
        #ifdef ASKED
        printf("Asked for permission by : %d\n",siginfo->si_pid);
        #endif // ASKED

        if(asked>=askedReq)
        {
            kill((*(childInfBuff+chInfIndex)).m_PID,SIGUSR1);
            #ifdef DEBUG
                printf("sending signal from interrupt to %d\n",(*(childInfBuff+chInfIndex)).m_PID);
            #endif // DEBUG
        }
        chInfIndex++;
        asked++;
    }// end if(fMother)
    else
    {
        #ifdef DEBUG
            printf("Permission Aquired \n");
        #endif // DEBUG
        fPermGranted=true;
    }//end else
}


void hndlSIGRT(int sig,siginfo_t *siginfo,void* context)
{
    if(fMother)
    {
        #ifdef REAL_TIME_SGI_REC
            printf("Recieved real time signal of number SIGRTMIN+%d from %d\n", (sig - SIGRTMIN),siginfo->si_pid);
        #endif // REAL_TIME_SGI_REC
        for(int i=0;i<chInfIndex;i++)
        {
            if( (*(childInfBuff+i)).m_PID==siginfo->si_pid)
            {
                (*(childInfBuff+i)).m_fEnded=true; // set to be waited for;
            }
        }
    }
    else
    {
        ;// do nothing
    }
}



void childLive()
{
    char secondChance=true;
    fMother=false;
    #ifdef CREATE
        printf("Created new thread %d\n",getpid());
    #endif // CREATE
    int tooSleepS=rand()%11;
    sleep(tooSleepS);
    kill(getppid(),SIGUSR1);

    time_t current=time(NULL);
    time_t end=current+15;
    while(!fPermGranted)
    {
        current=time(NULL);
        if(current>=end)
        {
            if(secondChance==true)
            {
                printf("\nreasking for permission\n");
                current+=2;
                kill(getppid(),SIGUSR1);
                secondChance=false;
            }
            else
            {
                printf("\nDying without permission \n");
                break;
            }
        }
    }
    int randSignal=rand()%32;
    kill(getppid(),SIGRTMIN+randSignal);
    #ifdef DEBUG
    printf("exiting with %d\n",randSignal);
    #endif // DEBUG
    exit(tooSleepS);
}


// arg  are n k
int main(int args, char * argv[])
{
    #ifdef DEBUG
    printf("I'm the nightmother and my PID is %d\n\n\n\n",getpid());
    #endif // DEBUG
    srand(time(NULL));
    int n=4;
    askedReq=2;
    if(3==args)
    {
        n=atoi(argv[1]);
        askedReq=atoi(argv[2]);
    }
    if(askedReq>n)
    {
        printf("nie mozemy czekac na wiecej niz bedzie!\n");
        askedReq=n;
    }
    struct childInf foo[n];
    childInfBuff=foo;// PID BUFF init;

    for(int i=0;i<n;i++)
    {
        foo[i].m_PID=0;
        foo[i].m_fEnded=false;
    }


    struct sigaction act1;
    act1.sa_flags=SA_SIGINFO;
    sigemptyset(&act1.sa_mask);
    act1.sa_sigaction=hndlSIGINT;
    sigaction(SIGINT,&act1,NULL);

    struct sigaction act2;
    act2.sa_flags=SA_SIGINFO;
    sigemptyset(&act2.sa_mask);
    act2.sa_sigaction=hndlSIGUSR1;
    sigaction(SIGUSR1,&act2,NULL);

    struct sigaction actRT;
    actRT.sa_flags=SA_SIGINFO;
    sigemptyset(&actRT.sa_mask);
    actRT.sa_sigaction=hndlSIGRT;
    for(int i= SIGRTMIN;i<=SIGRTMAX;i++)
    {
        sigaction(i,&actRT,NULL);
    }
    //sleep(1);
    int PID;
    for(int i=0;i<n;i++)
    {

        //printf("\n\nhuehueuheu\n\n");
        srand(rand());
        PID=fork();
        if(0==PID)
        {
            fMother=false;
            childLive();
        }
        //sleep(1);
    }


    int childrenLiving=n;
    int childToWaitFor=0;
    int returnVal;

    while(asked<askedReq)
    {
        pause();
    }

    for(int i=0;i<askedReq;i++)
    {
        kill((*(childInfBuff+i)).m_PID,SIGUSR1);
        #ifdef DEBUG
        printf("sending signal to process %d\n",(*(childInfBuff+i)).m_PID);
        #endif // DEBUG
    }
    //sleep(1);
    while(childrenLiving>0)
    {
        sleep(1);
        for(int i=0;i<n;i++)
        {
            #ifdef DEBUG
            printf("magiczna tablica %d, PID : %d, fEnded : %d\n",i,(*(childInfBuff+i)).m_PID,(*(childInfBuff+i)).m_fEnded);
            #endif // DEBUG
        }

        for(int i=0;i<chInfIndex;i++)
        {
            if((*(childInfBuff+i)).m_fEnded==true)
            {
                kill((*(childInfBuff+i)).m_PID,SIGUSR1);
                (*(childInfBuff+i)).m_fEnded=false;
                childToWaitFor=(*(childInfBuff+i)).m_PID;
                break;
            }
        }
        if(childToWaitFor)
        {
            waitpid(childToWaitFor,&returnVal,0);
            #ifdef CHILD_DIES
                printf("Children finished it's life pid:%d and slept for %ds\n",childToWaitFor,WEXITSTATUS(returnVal));
            #endif // CHILD_DIES
            childrenLiving--;
            childToWaitFor=0;
        }

    }


    return 0;
}
