#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "matrix.h"
#include <math.h>
#include <pthread.h>



typedef struct
{
    IntMatrix* sc;
    DoubleMatrix* filter;
    IntMatrix* outcome;
    int startRow;
    int endRow;
    char fReady;
}myThreadFuncData;



int max(int a, int b)
{
    if(a>b) return a;
    return b;
}

char countXY(IntMatrix* sc, DoubleMatrix* filter,IntMatrix* outcome,int x, int y)
{
    int c=filter->m_xSize;
    int i;
    int j;
    double dbToWrite=0.0;
    for(i=1;i<=c;i++)
    {
        for(j=1;j<=c;j++)
        {
            dbToWrite+=readIntMatrix(sc,(max(1,x-ceil(c/2.0)+i)-1),(max(1,y-ceil(c/2) + j)-1))*readDoubleMatrix(filter,i-1,j-1);
        }
    }
    printf("%f ",dbToWrite);
    if(dbToWrite<0) dbToWrite=255 + dbToWrite;
    if(writeIntMatrix(outcome,x,y,round(dbToWrite)));
    return true;
}


char saveImage(IntMatrix* imageMatrix,const char* filename)
{
    FILE* file=fopen(filename,"w");
    if(file==NULL) return returnWithError("Could not create output file");
    char line[256];
    sprintf(line,"P2\n");
    if(1!=fwrite(line,strlen(line),1,file)) return returnWithError("error on wite file");
    int iX=imageMatrix->m_xSize;
    int iY=imageMatrix->m_ySize;
    sprintf(line,"%d %d\n255\n",iY,iX);
    if(1!=fwrite(line,strlen(line),1,file)) return returnWithError("error on wite file");
    // since standard says no line should be onger than 70 cahracters, since our numbers are up to 3 digits, that's 4 with space
    // we'll put 16 numbers per line;
    int iNumbersCounter=0;
    int i,j;
    line[0]='\0';
    char chTmp[10];
    for(i=0;i<iX;i++)
    {
        for(j=0;j<iY;j++)
        {
            if(iNumbersCounter>=16)
            {
                line[strlen(line)-1]='\n';// change last space into '\n'
                if(1!=fwrite(line,strlen(line),1,file)) return returnWithError("error on wite file");
                iNumbersCounter=0;
                line[0]='\0';//clear line
                line[1]='\0';
            }
            sprintf(chTmp,"%d ",readIntMatrix(imageMatrix,i,j));
            strcat(line,chTmp);
            iNumbersCounter++;
        }
    }
    // since iNumbersCounter CANNOT be 0;
    line[strlen(line)-1]='\n';// change last space into '\n'
    if(1!=fwrite(line,strlen(line),1,file)) return returnWithError("error on write file");
    if(fclose(file)) return returnWithError("unable to close file");
    return true;
}


char calculate(IntMatrix* sc, DoubleMatrix* filter,IntMatrix* outcome,int startRow, int endRow)// it will do from start to end with end row. Aka (... , 0, IntMatrix->m_xSize) will comlplete all
// IntMatrix Outomce has to be inncialized
{
    if(startRow<0) return returnWithError("startRow lesser than 0");
    if(startRow>endRow) return returnWithError("startRow greater  than End");
    if(endRow>sc->m_xSize) return returnWithError("endRow out of scope");
    int i;
    int j;
    for(i=startRow;i<endRow;i++)
    {
        for(j=0;j<sc->m_ySize;j++)
        {
            if(!countXY(sc,filter,outcome,i,j)) return returnWithError("fail on countXY");
        }
    }
    return true;
}


void* myThreadFunc(void* input_myThreadFuncData)
{
    myThreadFuncData* in=input_myThreadFuncData;
    if(calculate(in->sc, in->filter,in->outcome,in->startRow,in->endRow))
    {
        in->fReady=true;
        return NULL;
    }
    else
    {
        printf("Fatal error occured during calculating outcome. Exitting");
        raise(9);
        return NULL;
    }

}











int main(int args, char* argv[])
{
    char* srcFileName,*filterFileName,*outComeFileName;
    int iThreadNumber=1;
    char srcFileNameBuff[100];
    sprintf(srcFileNameBuff,"test.pgm");
    char filterFileNameBuff[100];
    sprintf(filterFileNameBuff,"testFilter");
    char outComeFileNameBuff[100];
    sprintf(outComeFileNameBuff,"testOut.pgm");
    if(args==5)
    {
        iThreadNumber=atoi(argv[1]);
        srcFileName=argv[2];
        filterFileName=argv[3];
        outComeFileName=argv[5];
    }
    else
    {
        srcFileName=srcFileNameBuff;
        filterFileName=filterFileNameBuff;
        outComeFileName=outComeFileNameBuff;
    }

    FILE* file =fopen(srcFileName,"r");
    if(file==NULL)
    {
        printf("could not open read file:'%s'\n",srcFileName);
        return -1;
    }
    IntMatrix inputMatrix;
    printf("trying to read inputMatrix with outcome:%d\n",readAsciInt(&inputMatrix,file));
    printIntMatrix(&inputMatrix);
    DoubleMatrix filterMatrix;
    printf("vefore opening filer\n");
    FILE* filterFile= fopen(filterFileName,"r");
    if(filterFile==NULL)
    {
        printf("open failed on filter file:'%s'\n",filterFileName);
        return -1;
    }
    printf("\n\n\ntrying to read double with outcome:%d\n",readAsciDouble(&filterMatrix,filterFile));
    printDoubleMatrix(&filterMatrix);
    printf("\n\n\ndum dum dum\n\n\n\n");

    IntMatrix outMatrix;
    initIntMatrix(&outMatrix,inputMatrix.m_xSize,inputMatrix.m_ySize);

    /*
    myThreadFuncData t1;
    t1.filter=&filterMatrix;
    t1.sc=&inputMatrix;
    t1.outcome=&outMatrix;
    t1.startRow=0;
    t1.endRow=outMatrix.m_xSize;
    t1.fReady=false;
    if(!myThreadFunc(&t1))
    {
        printf("error on calculating result\n");
        return -1;
    }
    */
    int i;
    myThreadFuncData* dataTab[iThreadNumber];
    for(i=0;i<iThreadNumber;i++)
    {
        myThreadFuncData* data= malloc(sizeof(myThreadFuncData));
        if(data==NULL)
        {
            printf("malloc failed\n");
            return -1;
        }
        data->filter=&filterMatrix;
        data->sc=&inputMatrix;
        data->outcome=&outMatrix;
        data->startRow=0;
        data->endRow=outMatrix.m_xSize;
        data->fReady=false;
        pthread_t threadID;
        dataTab[i]=data;
        if(0!=pthread_create(&threadID,NULL,myThreadFunc,(void*)data)) return -1;
    }

    for(i=0;i<iThreadNumber;i++)
    {
        while(!dataTab[i]->fReady); // loop until thread finishes;
        free(dataTab[i]);
    }


    saveImage(&outMatrix,outComeFileName);


    return 0;
}
