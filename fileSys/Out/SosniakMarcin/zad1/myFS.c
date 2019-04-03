#include "myFS.h"
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#define true 1
#define false 0


void randBlock(char* target,int blockSize)
{
    for(int i=0;i<blockSize;i++)
        *(target+i)=rand()%256;
}



char generate(char * path, int size, int blockSize)
{
    int fileDes=open(path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR  | S_IWUSR | S_IROTH);
    if(fileDes < 0) return false;
    char testBlock[blockSize];

    for(int k=0;k<size;k++)
    {
        randBlock(testBlock,blockSize);
        if(write(fileDes,testBlock,blockSize) < blockSize )
            return false;
    }
    if(close(fileDes)) return false; // if returns non zero save failed
    return true;
}



char copySys(char * pathSRC,char * pathTRGT, int size, int blockSize)
{
    int fileDtarget=open(pathTRGT,O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR  | S_IWUSR | S_IROTH);
    if(fileDtarget < 0) return false;
    int fileDsrc= open(pathSRC, O_RDONLY);
    if(fileDsrc < 0) return false;
    char tmpBl[blockSize];
    for(int i=0;i<size;i++)
    {
        if( read(fileDsrc,tmpBl,blockSize) <  blockSize )
        {
            close(fileDsrc);
            close(fileDtarget);
            return false;
        }

        if (write(fileDtarget,tmpBl,blockSize) < blockSize)
        {
            close(fileDsrc);
            close(fileDtarget);
            return false;
        }

    }
    if(close(fileDsrc) || close(fileDtarget)) return false;// if any is non zero copy failed
    return true;
 }



 char copyLib(char * pathSRC,char * pathTRGT, int size, int blockSize)
 {
    FILE* fSrc=fopen(pathSRC,"r");
    FILE* fTrgt=fopen(pathTRGT,"w");
    if( fSrc == NULL || fTrgt == NULL) return false;
    char tmpBl[blockSize];
    for(int i=0;i<size;i++)
    {
        if(  fread(tmpBl,blockSize,1,fSrc)<  1)
        {
            fclose(fSrc);
            fclose(fTrgt);
            return false;
        }

        if ( fwrite(tmpBl,blockSize,1,fTrgt)< 1)
        {
            fclose(fSrc);
            fclose(fTrgt);
            return false;
        }

    }
    if(fclose(fSrc)) return false;// if any is non zero copy failed
    if(fclose(fTrgt)) return false;// if any is non zero copy failed
    tmpBl[0]++;
    return true;
 }







 char insertSys(int fileDes,int iPosToIns, int iPosEnd,int iBlockSize, char* cbToIns)
 {
    if(iPosEnd<iPosToIns) return false;
    if(lseek(fileDes,iPosToIns*iBlockSize,SEEK_SET)<0) return false;
    char cbToCopy[iBlockSize];
    char cbBackUp[iBlockSize];
    for(int i=0;i<iBlockSize;i++)
    {
        cbToCopy[i]=*(cbToIns+i);
    }

    for(int i=iPosToIns;i<=iPosEnd;i++)
    {
        if(lseek(fileDes,i*iBlockSize,SEEK_SET)<0) return false; // return pointer to posioton
        if(read(fileDes,cbBackUp,iBlockSize) < iBlockSize) return false;
        if(lseek(fileDes,-iBlockSize,SEEK_CUR)<0) return false; // return pointer to posioton
        // an error cannot occur since pointer has to be even or greater than iBlockSize, or insert will fail
        if(write(fileDes,cbToCopy,iBlockSize) < iBlockSize) return false;
        for(int k=0;k<iBlockSize;k++)
        {
            cbToCopy[k]=cbBackUp[k];
        }
    }



    return true;
 }






char sortSys(char* pathSRC, int iSize, int iBlockSize)
{
    int fileDes=open(pathSRC,O_RDWR);
    if(fileDes<0) return false;
    int iSorted=1;
    if(iSize <= 1) return true;
    char cbCurrentBlock[iBlockSize];
    unsigned char ucCompared;
    unsigned char ucTested;
    while(iSorted < iSize)
    {
        if(lseek(fileDes,iSorted*iBlockSize,SEEK_SET) < 0 )  return false;
        if(read(fileDes,cbCurrentBlock,iBlockSize) < iBlockSize) return false;
        ucTested=cbCurrentBlock[0];
        for(int k=0;k<iSorted;k++)
        {
            if(lseek(fileDes,k*iBlockSize,SEEK_SET) < 0 )  return false;
            if(read(fileDes,&ucCompared,1) < 1) return false;
            if(ucCompared > ucTested)
            {
                if(!insertSys(fileDes,k,iSorted,iBlockSize,cbCurrentBlock)) return false;
                break;
            }
        }
        iSorted++;
    }
    if(close(fileDes)<0) return false;
    return true;


}






 char insertLib(FILE* fTarget,int iPosToIns, int iPosEnd,int iBlockSize, char* cbToIns)
 {
    if(iPosEnd<iPosToIns) return false;
    if(fseek(fTarget,iPosToIns*iBlockSize,SEEK_SET)) return false; //if not 0 return false
    char cbToCopy[iBlockSize];
    char cbBackUp[iBlockSize];
    for(int i=0;i<iBlockSize;i++)
    {
        cbToCopy[i]=*(cbToIns+i);
    }

    for(int i=iPosToIns;i<=iPosEnd;i++)
    {
        if(fread(cbBackUp,iBlockSize,1,fTarget) < 1) return false;
        if(fseek(fTarget,-iBlockSize,SEEK_CUR)<0) return false; // return pointer to posioton
        // an error cannot occur since pointer has to be even or greater than iBlockSize, or insert will fail
        if(fwrite(cbToCopy,1,iBlockSize,fTarget) < 1) return false;
        for(int k=0;k<iBlockSize;k++)
        {
            cbToCopy[k]=cbBackUp[k];
        }
    }



    return true;
 }






char sortLib(char* pathSRC, int iSize, int iBlockSize)
{
    FILE* fTarget=fopen(pathSRC,"r+");
    if(fTarget == NULL) return false;
    int iSorted=1;
    if(iSize <= 1) return true;
    char cbCurrentBlock[iBlockSize];
    unsigned char ucCompared;
    unsigned char ucTested;
    while(iSorted < iSize)
    {
        if(fseek(fTarget,iSorted*iBlockSize,SEEK_SET) )  return false;
        if(fread(cbCurrentBlock,iBlockSize,1,fTarget) < 1) return false;
        ucTested=cbCurrentBlock[0];
        for(int k=0;k<iSorted;k++)
        {
            if(fseek(fTarget,k*iBlockSize,SEEK_SET)  )  return false;
            if(fread(&ucCompared,1,1,fTarget) < 1) return false; // read one byte
            if(ucCompared > ucTested)
            {
                if(!insertLib(fTarget,k,iSorted,iBlockSize,cbCurrentBlock)) return false;
                break;
            }
        }
        iSorted++;
    }
    if(fclose(fTarget)<0) return false;
    return true;


}































