#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <signal.h>

#define true 1
#define false 0


char fStop=false;
int displayPID=-1;


void displayTime()
{

    int iPID;
    iPID=fork();
    if(0==iPID)
    {

        execl("./script",NULL);

        exit(1);
    }
    displayPID=iPID;
    return;
}


void reactSigInt()
{
    printf("\notrzymano sygnal SIGINT\n");
    if(displayPID!=-1)
        kill(displayPID,SIGKILL);
    exit(1);
}

void reactSigStop ()
{
    if(displayPID==-1)
    {
        printf("\npotomek nie zdefiniowany, zamkniecie awaryjne\n");
        exit(1);
    }
    if(!fStop)
    {
        fStop=true;
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
        kill(displayPID,SIGSTOP);
        return;
    }
    kill(displayPID,SIGCONT);
    fStop=false;
    printf("\n");
}



int main()
{

    struct sigaction act;
    act.sa_handler=reactSigStop;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGTSTP,&act,NULL);
    signal(SIGINT,reactSigInt);

    displayTime();

    while(1)
    {
        pause();
        //sleep(1);
    }

    return 0;
}
