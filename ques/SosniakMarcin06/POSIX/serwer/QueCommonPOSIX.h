#ifndef QUECOMMONPOSIX_H_INCLUDED
#define QUECOMMONPOSIX_H_INCLUDED


#ifndef true
    #define true 1
#endif // true

#ifndef false
    #define false 0
#endif // true

#define PROJ 'V'
#define MIRROR 11
#define CALC 12
#define TIME 13
#define END 14
#define STOP 15
#define START 16
#define MAX_COMM_LEN 128
#define MAX_SEND_LEN 120

#define MAX_MESSAGE_COUNT 10;

#define MAIN_QUE_NAME "/QueTest000000"


/*

message XXXXXX YYYYYYYYYYYYYY...
XXXXXX 6 digit PID followed by single space. Rest is \0 terminated string
Serwer wyslya 6 ZNAKOWE PID potem jedna spacje, potem klucz do message Queue jako std String, means terminated by null
STOP zawire tylko 6ZNAKOWE PID terminowane NULLEM;
*/


/*
POSIX Note: Que struct wil always be :  /tmp/QueTest/qXXXXXX  where XXXXXX is current progrma PID;
with the execption of /tmp/QueTest/q000000 which is the main q

Also: -lrt is needed for compiling

*/

typedef struct{
    unsigned char iType;
    char msgBuff[MAX_COMM_LEN];
}my_msg;






#endif // QUECOMMONPOSIX_H_INCLUDED
