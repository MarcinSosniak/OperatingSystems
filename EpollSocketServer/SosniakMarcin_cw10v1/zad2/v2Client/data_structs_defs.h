#ifndef DATA_STRUCTS_DEFS_H_INCLUDED
#define DATA_STRUCTS_DEFS_H_INCLUDED


#ifndef true
    #define true 1
#endif // true

#ifndef false
    #define false 0
#endif // false

#include <stdlib.h>



#define MAX_CLIENT_NAME_LEN 20
#define MAX_MESSAGE_LENGHT 32
#define MAX_UNIX_SOCKET_NAME_LEN 92


typedef unsigned char byte;
typedef char bool;
/*
|-----------------------------------|
|        data types defs            |
|-----------------------------------|
*/
// Operators ; contain void* as double[2] unless specified otherwise
#define EMPTY_OPERATOR 0
#define OPERATOR_ADD 100
#define OPERATOR_SUB 101
#define OPERATOR_MUL 102
#define OPERATOR_DIV 103
// Orders
#define EMPTY_MESSAGE 0
#define I_LIVE 1 // message is const char* string with name
#define ACCEPTED 2 // message is NULL
#define DECLINED 3 // message is NULL
#define ANSWER 4 // message is double[1]
#define LOGOUT 5 // message is const char* string with name
#define POKE 6 // message is NULL





/*
|--------------END------------------
*/




typedef struct
{
    byte m_type;
    short m_length;
    int m_operationId; // id number of given task, not Type, should not be touched by client
    int m_iClientId;// client should not touch it or send the same number w/e
    byte  m_message[MAX_MESSAGE_LENGHT];
}message;

/*

*/

void clearMessage(message* me);






#endif // DATA_STRUCTS_&_DEFS_H_INCLUDED
