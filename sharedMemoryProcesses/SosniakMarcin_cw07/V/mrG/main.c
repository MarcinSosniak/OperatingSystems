#include <stdio.h>
#include <stdlib.h>
#include "shStruct.h"
#include "sys/shm.h"
#include "sys/ipc.h"
#include "sys/sem.h"
#include "sys/stat.h"
#include "unistd.h"
#include "signal.h"

#include "sys/mman.h"
#include "semaphore.h"
#include "fcntl.h"

//#define DEBUG 1

int shmID=0;
sem_t* semP=NULL;
struct shMemSt * shmP;

void die();

void semUp()
{
    if(sem_post(semP)<0 )
    {
        perror("error on locking semaphore");
        die();
    }

    #ifdef DEBUG
    printf("semaphore Up\n");
    #endif // DEBUG
}

void semDown()// lock
{
    if(sem_wait(semP)<0)
    {
        perror("error on locking semaphore");
        die();
    }


    #ifdef DEBUG
    printf("semaphore Down\n");
    #endif // DEBUG
}

void signalHNDL(int sig)
{
    printf("signal interrupt");
    die();
}



void die()
{
    int iRet;
    if(shmP!=NULL && (long long)shmP!=(-1))
    {
        iRet=munmap(shmP,sizeof(struct shMemSt));
        if(iRet<0) perror("failed on munmap>");
    }
    if(shm_unlink(SHM_NAME)<0)
    {
        perror("fail on deleting Shared Memory Fragment");
    }
    if(sem_close(semP)<0) perror("fail on sem_close");
    if(sem_unlink(SEM_NAME)<0) perror("fail on sem_unlink");

    exit(0);
}

void printWithTime(char * input)
{
    struct timespec tm;
    clock_gettime(CLOCK_REALTIME,&tm);
    printf("%s[%ld,%ld]\n",input,tm.tv_sec,tm.tv_nsec);
}


void mrGlive()
{
    char inBrrrr=false;
    while(true)
    {
        //
        semDown();
        //
        if(inBrrrr==true) // finish brrrr
        {
                inBrrrr=false;
                char sTmp[200];
                sprintf(sTmp,"finishing brrrrr %d",shmP->iSeat);
                shmP->iSeat=0; // clear sit
                //
                semUp();
                //
                printWithTime(sTmp);
                randomUsleep();
                continue;
        }

        if(shmP->bGsleep==true)
        {
            if(shmP->bGwake==true)
            {// wake
                shmP->bGsleep=false;
                shmP->bGwake=false;

                //here client already sat in chair without invitation

                char sTmp[200];
                inBrrrr=true;
                sprintf(sTmp,"starting brrrrrr %d",shmP->iSeat);
                //
                semUp();
                //
                printWithTime("mrG wakes up>");
                printWithTime(sTmp);
                randomUsleepT(10);
                continue;
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
        if(shmP->bNewClient==false)// when mr G had not yet inivited, or noone responeded
        {
            if(queGetCount(&(shmP->wRoom))<=0 && shmP->invitedPID==0) // nothing to be done
            {
                printWithTime("mrG going to sleep>");
                shmP->bGsleep=true;
                //
                semUp();
                //
                randomUsleep();
                continue;
            }
            if(shmP->invitedPID==0)
            {
                int pid=queGet(&(shmP->wRoom));
                if(pid<=0)
                {
                    printf("fatal integration error, on getting new pid to invite\n");
                    die();
                }
                char sTmp[200];
                sprintf(sTmp,"inviting new %d",pid);

                shmP->invitedPID=pid;
                //
                semUp();
                //
                printWithTime(sTmp);
                randomUsleep();
                continue;
            }
            // if anyone is inivted, he did not yet respond;
            //
            semUp();
            //
            randomUsleep();
            continue;
        }

        if(shmP->bNewClient==true) // invited and responded
        {
                shmP->iSeat=shmP->invitedPID;
                shmP->bNewClient=false;
                shmP->invitedPID=0;
                inBrrrr=true;

                char sTmp[200];
                sprintf(sTmp,"beging brrrr%d",shmP->iSeat);


                //
                semUp();
                //
                printWithTime(sTmp);
                randomUsleep();
                continue;
        }

        printf("A fatal error occured and noone of conditions were met dumping SHM now\n");
        printf("bGsleep=%d,bGwake=%d,bNewClient=%d,invitedPID=%d,iSeat=%d\n",shmP->bGsleep,shmP->bGwake,shmP->bNewClient,shmP->invitedPID,shmP->iSeat);
        char chBuff[1000];
        queToString(&(shmP->wRoom),chBuff);
        printf("%s\n",chBuff);
        die();

    }


}








int main()
{
    signal(SIGTERM,signalHNDL);
    signal(SIGINT,signalHNDL);
    // create shared space
    shmID=  shm_open(SHM_NAME,O_CREAT | O_EXCL | O_RDWR , 0666);
    if(shmID<0)
    {
        perror("on creation SHM>");
        exit(-1);
    }
    int iRet=ftruncate(shmID,sizeof(struct shMemSt));
    if (iRet<0)
    {
        perror("fail on ftruncate>");
        exit(-1);
    }
    shmP=mmap(NULL,sizeof(struct shMemSt),PROT_READ | PROT_WRITE | PROT_EXEC,MAP_SHARED,shmID,0);
    if((long long)shmP<0)
    {
        perror("on opening shm>");
    }
    //Create space;

    //init sleep
    initRandomSleep();
    struct shMemSt shMemToBeCopied;
    shMemStInit(&shMemToBeCopied);
    //

    //copy structure to shared space
    memcpy(shmP,&shMemToBeCopied,sizeof(struct shMemSt));

    // create sem
    semP= sem_open(SEM_NAME,O_CREAT | O_EXCL,0666,1);
    if(semP==SEM_FAILED)
    {
        perror("fail on setting sem val");
        die();
    }

    // finished init
    mrGlive();



    die();
    printf("Hello world!\n");
    return 0;
}
