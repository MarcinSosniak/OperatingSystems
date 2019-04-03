#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <locale.h>
#include <signal.h>

#define true 1
#define false 0


char fStop=false;


void displayTime()
{

    time_t crTime;
    time(&crTime);
    struct tm *tmTime;
    char out[100];
    setlocale (LC_ALL, "pl_PL");
    tmTime=localtime(&crTime);
    strftime(out,99,"%Y-%m-%d %H:%M:%S",localtime(&crTime));
    printf("%s\n",out);
}


void reactSigInt()
{
    printf("\notrzymano sygnal SIGINT\n");
    exit(1);
}

void reactSigStop ()
{
    if(!fStop)
    {
        fStop=true;
        printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C - zakonczenie programu\n");
        return;
    }
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

    while(1)
    {
        if(!fStop)displayTime();
        sleep(1);
    }

    return 0;
}
