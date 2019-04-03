#ifndef QUECOMMON_H_INCLUDED
#define QUECOMMON_H_INCLUDED

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
#define MAX_COMM_LEN 1024


/*

message XXXXXX YYYYYYYYYYYYYY...
XXXXXX 6 digit PID followed by single space. Rest is \0 terminated string
Serwer wyslya 6 ZNAKOWE PID potem jedna spacje, potem klucz do message Queue jako std String, means terminated by null
STOP zawire tylko 6ZNAKOWE PID terminowane NULLEM;
*/


typedef struct{
    long iType;
    char msgBuff[MAX_COMM_LEN];
}my_msg;




#endif // QUECOMMON_H_INCLUDED
