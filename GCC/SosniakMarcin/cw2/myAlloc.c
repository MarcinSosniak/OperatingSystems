#include "myAlloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define true 1
#define false 0



int diff(int a, int b)
{
    int c= a-b;
    if(c<0) return -c;
    return c;
}



extern struct allocData1 aD1={NULL,0,0};
extern struct allocData2 aD2={NULL,0,0};


char alloc1(int size, int blockSize)
{
    if( aD1._tblP!=NULL) return false;
    void* ptr=calloc(size,blockSize);
    if(ptr==NULL) return false;
    aD1._tblP=ptr;
    aD1._iSize=size;
    aD1._iBlockSize=blockSize;
    return true;
}


char free1()
{
    if(aD1._tblP==NULL) return false;
    free(aD1._tblP);
    aD1._tblP=NULL;
    aD1._iSize=0;
    aD1._iBlockSize=0;
    return true;
}

char add1(int index, char* data)
{
    if(aD1._tblP==NULL) return false;
    char* ptr=aD1._tblP;
    ptr+=(index*aD1._iBlockSize);
    void* test=memcpy( ptr , data, aD1._iBlockSize);
    if(test==NULL) return false;
    return true;
}

char remove1(int index)
{
    if(aD1._tblP==NULL) return false;
    char* ptr=aD1._tblP;
    ptr+=(index*aD1._iBlockSize);
    for(int i=0;i<aD1._iBlockSize;i++)
    {
        *ptr='\0';
        ptr++;
    }
    return true;
}

int search1(int index)
{
    int iSearchedSize=0;
    char* ptr=aD1._tblP;
    ptr+=(index*aD1._iBlockSize);
    for(int k=0;k<aD1._iBlockSize;k++)
    {
        iSearchedSize+=*ptr;
        ptr++;
    }
    ptr=aD1._tblP;
    int tmpAmount=0;
    int iWinId=-1;
    int iMinDiff = 2147483646;
    for(int i=0;i<aD1._iSize;i++)
    {
        if(i==index)
        {
            ptr+=aD1._iBlockSize;
            continue;
        }
        tmpAmount=0;
        for(int k=0;k<aD1._iBlockSize;k++)
        {
            tmpAmount+=*ptr;
            ptr++;
        }


        if(iMinDiff>diff(tmpAmount,iSearchedSize))
        {
            iMinDiff=diff(tmpAmount,iSearchedSize);
            iWinId=i;
        }
    }
    return iWinId;
}


char* get1(int index)
{
    if(aD1._tblP ==NULL) return NULL;
    return ((char*) aD1._tblP)+(index*aD1._iBlockSize);
}




///////////////////////////////////////////////////////////////////////////////


char alloc2(int size, int blockSize)
{
    if( aD2._tblP!=NULL) return false;
    void* ptr=calloc((sizeof(void *)),size);
    if(ptr==NULL) return false;
    aD2._tblP=ptr;
    aD2._iSize=size;
    aD2._iBlockSize=blockSize;
    char ** clearP=ptr;
    for(int i=0;i<size;i++)
    {
        *clearP=NULL;
        clearP++;
    }
    return true;
}


char free2()
{
    if(aD2._tblP==NULL) return false;
    char **ptr=aD2._tblP;
    for(int i=0;i<aD2._iSize;i++)
    {
        if(*ptr!=NULL)
            free(*ptr);
        ptr++;
    }

    free(aD2._tblP);
    aD2._tblP=NULL;
    aD2._iSize=0;
    aD2._iBlockSize=0;
    return true;
}



char add2(int index, char* data)
{
    if( aD2._tblP==NULL) return false;
    char** ptr=aD2._tblP;
    ptr+=index;
    char* target=malloc(aD2._iBlockSize);
    if(target==NULL) return false;
    *ptr=target;
    void* testP;
    testP=memcpy(target,data,aD2._iBlockSize);
    if(testP!=NULL) return true;
    *ptr=NULL;
    return false;

}



char remove2(int index)
{
    if( aD2._tblP==NULL) return false;
    char ** ptr=aD2._tblP;
    ptr+=index;
    free(*ptr);
    *ptr=NULL;
    return true;
}


char* get2(int index)
{
    char** ptr=aD2._tblP;
    ptr+=index;
    if(ptr==NULL) return NULL;
    return *ptr;
}



int search2(int index)
{
    int iSearchedSize=0;
    char** ptr=aD2._tblP;
    ptr+=index;
    char *target;
    target=*ptr;
    for(int k=0;k<aD2._iBlockSize;k++)
    {
        iSearchedSize+=*target;
        target++;
    }
    ptr=aD2._tblP;
    int tmpAmount=0;
    int iWinId=-1;
    int iMinDiff = 2147483646;
    for(int i=0;i<aD2._iSize;i++)
    {
        if(i==index)
        {
            continue;
        }

        target=*(ptr+i);
        tmpAmount=0;

        for(int k=0;k<aD2._iBlockSize;k++)
        {
            tmpAmount+=*target;
            target++;
        }

        if(iMinDiff>diff(tmpAmount,iSearchedSize))
        {
            iMinDiff=diff(tmpAmount,iSearchedSize);
            iWinId=i;
        }
    }
    return iWinId;
}
