#include <stdio.h>
#include <stdlib.h>
#include "shStruct.h"
#include "sys/shm.h"
#include "sys/ipc.h"
#include "sys/sem.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "unistd.h"
#include "signal.h"

//#define DEBUG

int shmID=0;
int semID=0;
int iMyPid=0;
struct shMemSt * shmP;


void printWithTime(char * input)
{
    struct timespec tm;
    clock_gettime(CLOCK_REALTIME,&tm);
    printf("%s[%d,%d]\n",input,tm.tv_sec,tm.tv_nsec);
}


void printWithTimePid(char * input, int pid)
{
    struct timespec tm;
    clock_gettime(CLOCK_REALTIME,&tm);
    printf("%d>%s[%d,%d]\n",pid,input,tm.tv_sec,tm.tv_nsec);
}

void die()
{
    exit(-1);
}

void dieAndPrint()
{
    printf("A fatal error occured and noone of conditions were met dumping SHM now\n");
    printf("bGsleep=%d,bGwake=%d,bNewClient=%d,invitedPID=%d,iSeat=%d\n",shmP->bGsleep,shmP->bGwake,shmP->bNewClient,shmP->invitedPID,shmP->iSeat);
    char chBuff[1000];
    queToString(&(shmP->wRoom),chBuff);
    printf("%s\n",chBuff);
    die();
}





void semUp()
{

    struct sembuf sop;
    sop.sem_flg = 0;
    sop.sem_num = 0;
    sop.sem_op = 1;
    if(semop(semID,&sop,1) == -1 )
    {
        perror("error on locking semaphore");
        die();
    }

    #ifdef DEBUG
    printf("semaphore Up for %d\n",iMyPid);
    #endif // DEBUG
}

void semDown()// lock
{
    struct sembuf sop;
    sop.sem_flg = 0;
    sop.sem_num = 0;
    sop.sem_op = -1;
    if(semop(semID,&sop,1) == -1 )
    {
        perror("error on locking semaphore");
        die();
    }
    #ifdef DEBUG
    printf("semaphore Down for %d\n",iMyPid);
    #endif // DEBUG
}


void clientLive(int iCounter)
{   if(iCounter<=0) iCounter=2;
    iMyPid=getpid();

    char bGettingCut=false;
    char bInQue=false;
    char bInvited=false;
    for(;iCounter>0;iCounter--)
    {
        printf("%d> trying to get haricut #%d\n",iMyPid,iCounter);

        bGettingCut=false;
        bInQue=false;
        bInvited=false;
        while(true)
        {
            #ifdef DEBUG

            #endif // DEBUG
            semDown();
            if(bGettingCut==true)
            {

                #ifdef DEBUG
                printf("Trying to check for haircut%d\n",iMyPid);
                #endif // DEBUG
                if(shmP->iSeat!=iMyPid)// we have been kicked
                {
                    bGettingCut=false;
                    //
                    semUp();
                    //
                    printWithTimePid("brand new haircut",iMyPid);
                    break;
                }
                else
                {
                    //
                    semUp();
                    //
                    randomUsleep();
                    continue;
                }
            }//and if bGettingCut

            if(bInQue== false && bInvited==false && shmP->bGsleep== true && shmP->bGwake ==false ) // jesli spi i nikt go nie budzi
            {
                bGettingCut=true;
                shmP->iSeat=iMyPid;
                shmP->bGwake =true;
                printWithTimePid("waking up mr G>",iMyPid);
                printWithTimePid("takinig the seat>",iMyPid);
                //
                semUp();
                //
                randomUsleep();
                continue;
            }

            if(bInQue==false)
            {
                if(queAdd(&(shmP->wRoom),iMyPid)==true)
                {
                    bInQue=true;
                    //
                    semUp();
                    //
                    printWithTimePid("taking place in waiting room>",iMyPid);
                    randomUsleep();
                    continue;
                }
                else
                {
                    //
                    semUp();
                    //
                    bGettingCut=false;
                    bInQue=false;
                    bInvited=false;
                    printWithTimePid("leaving barbershop, gonna come back later",iMyPid);
                    randomUsleepT(50);
                    continue;
                }
            }
            if(bInQue==true && bInvited==false)
            {
                #ifdef DEBUG
                printf("trying to check if inivted %d\n",iMyPid);
                #endif // DEBUG
                if(shmP->invitedPID==iMyPid)
                {
                    bInvited=true; // no breakeroni so no semUpperoni
                }
                else
                {
                    //
                    semUp();
                    //
                    randomUsleep();
                    continue;

                }
            }
            if(bInQue==true && bInvited==true)
            {
                if(shmP->invitedPID!=iMyPid)
                {
                    semUp();
                    printf("integriry error on invitation get(not my PID on inv)\n");

                    #ifdef DEBUG
                    printf("%d reached end of while, should not happen,bGettingCut=%d,bInQue=%d,bInvited=%d\n",iMyPid,bGettingCut,bInQue,bInvited);
                    #endif // DEBUG
                    dieAndPrint();
                }
                if(shmP->iSeat!=0)
                {
                    semUp();

                    printf("integrity error on invitation get(seat not empty)\n");
                    #ifdef DEBUG
                    printf("%d reached end of while, should not happen,bGettingCut=%d,bInQue=%d,bInvited=%d\n",iMyPid,bGettingCut,bInQue,bInvited);
                    #endif // DEBUG

                    dieAndPrint();
                }
                shmP->bNewClient=true;
                bGettingCut=true;
                printWithTimePid("taking seat ",iMyPid);
                //
                semUp();
                //
                randomUsleep();
                continue;
            }

        #ifdef DEBUG
        printf("%d reached end of while, should not happen,bGettingCut=%d,bInQue=%d,bInvited=%d\n",iMyPid,bGettingCut,bInQue,bInvited);
        #endif // DEBUG
        }// end while
    randomUsleepT(15);
    }// end for
}


void mother(int iClientsCount,int iBrrCount)
{
    while(true)
    {
        int iPids[iClientsCount];
        int iPID;
        int i;
        int status;
        for(i=0;i<iClientsCount;i++)
        {
            iPID=fork();
            if(iPID==0)
            {
                printf("new processCreated %d\n",getpid());
                //sleep(1);
                clientLive(iBrrCount);
                die();
            }
            else
            {
                iPids[i]=iPID;
            }
        }
        sleep(1);
        for(i=0;i<iClientsCount;i++)
        {
            waitpid(iPids[i],&status,0);
        }
        printf("press 'y' and enter  if you wish to retry:[Y/N]");
        char chBuff[81];
        fgets(chBuff,80,stdin);
        printf("\n");
        if(chBuff[0]!='y')
            break;
    }
}



// liczbe klientow, liczba strzyzen
int main(int args, char* argv[])
{
    int iClientsCount=3;
    int iBrrCount=2;
    if(args==3 && atoi(argv[1])>0 && atoi(argv[2])>0)
    {
        iClientsCount=atoi(argv[1]);
        iBrrCount=atoi(argv[2]);
    }


    // init global
    iMyPid=getpid();
    // init SHM
    shmID=  shmget(ftok("/tmp",'a'), sizeof(struct shMemSt), 0);
    if(shmID<0)
    {
        perror("on opening SHM>");
        die();
    }
    shmP=shmat(shmID,NULL,0);
    if((long long)shmP<0)
    {
        perror("on opening shm>");
        die();
    }


    // init sem
    semID = semget(ftok("/tmp",'a'), 1 ,0);
    if (semID == -1)
    {
        perror("fail on opening semaphore");
        die();
    }

    mother(iClientsCount,iBrrCount);

    printf("Hello world!\n");
    return 0;
}
