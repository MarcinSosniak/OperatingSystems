#ifndef MYALLOC_H_INCLUDED
#define MYALLOC_H_INCLUDED

struct allocData1
{
    void* _tblP;
    int _iSize;
    int _iBlockSize;
};

struct allocData2
{
    void* _tblP;
    int _iSize;
    int _iBlockSize;
};

int diff(int,int);
char alloc1(int size,int blocksize);
char add1(int index, char* src);
char remove1(int index);
char* get1(int index);
char free1();
int search1(int index);


char alloc2(int size,int blocksize);
char add2(int index, char* src);
char remove2(int index);
char* get2(int index);
char free2();
int search2(int index);




#endif // MYALLOC_H_INCLUDED
