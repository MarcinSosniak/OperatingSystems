#ifndef SHSTRUCT_H_INCLUDED
#define SHSTRUCT_H_INCLUDED

#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#ifndef true
#define true 1
#endif // true

#ifndef false
#define false 0
#endif // false

#define QUE_SIZE 10

void initRandomSleep();
void randomUsleep();
void randomUsleepT(int times);





struct que{
    char bRdy;
    int iBeg;
    int iEnd;
    int iCount;
    int iQue[QUE_SIZE];
};

char queInit(struct que* q);
char queAdd(struct que* q,int iIn);
int queGet(struct que* q); // returns 0 if there is nothing;
int queGetCount(struct  que* q); // return -1 as error
void queToString(struct  que* q,char * chBuff);



struct shMemSt
{
    char bGsleep; // changed only by mr G himself
    char bGwake; // set by clients, reset by G
    char bNewClient;
    int invitedPID; // 0 when empty
    /*
    Idea; G pops Queue and pases to inivtes if he wishes to invite
    Client himself moves into chair, only when HE is invited;
    */
    int iSeat; // contains PID of client; 0 when empty;
    struct que wRoom;
};

char shMemStInit(struct shMemSt *);

/*
Client is state machine. If he moved himself into chair and later got kicked out of his it means he'd been cut


*/









#endif // SHSTRUCT_H_INCLUDED
