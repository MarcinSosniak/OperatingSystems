#ifndef SERVER_STRUCTS_H_INCLUDED
#define SERVER_STRUCTS_H_INCLUDED

#define MAX_ANSWER_LEN 255


typedef struct
{
    byte m_bOperation;
    double m_dArg1;
    double m_dArg2;
    int m_iTaskId;
}task;

void clearTask(task * );

typedef struct
{
    char toPrint[MAX_ANSWER_LEN+1];
    char fReady;
}toPrint;


#endif // SERVER_STRUCTS_H_INCLUDED
