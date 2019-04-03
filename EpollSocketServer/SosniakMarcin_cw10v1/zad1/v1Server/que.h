#ifndef QUE_H_INCLUDED
#define QUE_H_INCLUDED

#include "stdlib.h"
#include "data_structs_defs.h"
#include "server_structs.h"

#ifndef true
#define true 1
#endif // true

#ifndef false
#define false 0
#endif // false



#define QUE_SIZE 1000


typedef struct{
    char bRdy;
    int iBeg;
    int iEnd;
    int iCount;
    int iMaxSize;
    message iQue[QUE_SIZE];
}messageQue;

char msQueInit( messageQue* q);
char msQueAdd( messageQue* q,message iIn);
message msQueGet( messageQue* q); // returns null if there is nothing;
int msQueGetCount( messageQue* q); // return -1 as error
void msQueToString( messageQue* q,char * chBuff);
char msQueIsFull( messageQue* q);

typedef struct{
    char bRdy;
    int iBeg;
    int iEnd;
    int iCount;
    int iMaxSize;
    task iQue[QUE_SIZE];
}taskQue;

char tQueInit( taskQue* q);
char tQueAdd( taskQue* q,task iIn);
task tQueGet( taskQue* q); // returns null if there is nothing;
int tQueGetCount( taskQue* q); // return -1 as error
void tQueToString( taskQue* q,char * chBuff);
char tQueIsFull( taskQue* q);

#include "server.h"


#endif // QUE_H_INCLUDED
