#include "matrix.h"
char returnWithError(const char* in)
{
    printf("%s\n",in);
    return false;
}

char checkLine(char * line)
{
    int i=0;
    while(line[i]!='\0' && i<LINE_LEN)
    {
        if(line[i]=='\n') return true;
        i++;
    }
    return false;
}

char anyComments(char * line)
{
    int i=0;
    while(line[i]!='\0' && i<LINE_LEN)
    {
        if(line[i]=='#') return true;
        i++;
    }
    return false;
}


char initIntMatrix(IntMatrix * me,int x,int y)
{
    if( y <= 0 || x <= 0) return false;
    me->m_xSize=x;
    me->m_ySize=y;
    me->m_tab= malloc((sizeof (int))*x*y);
    if(me->m_tab == NULL)
    {
        me->m_xSize=0;
        me->m_ySize=0;
        return false;
    }
    return true;
}

char clearIntMatrix(IntMatrix * me)
{
    if(me->m_xSize<=0 || me->m_ySize <=0) return false;
    free(me->m_tab);
    return true;
}
int readIntMatrix(IntMatrix * me,int x,int y)
{
    if(me->m_xSize<=0 || me->m_ySize <=0 || me->m_tab==NULL || x >= me->m_xSize || y>= me->m_ySize || x < 0 || y < 0 ) return 0;
    return *(me->m_tab +(x * me->m_xSize) + y);
}
char writeIntMatrix(IntMatrix * me,int x,int y, int input)
{
    if(me->m_xSize<=0 || me->m_ySize <=0 || me->m_tab==NULL || x >= me->m_xSize || y>= me->m_ySize || x < 0 || y < 0 ) return false;
    *(me->m_tab +(x * me->m_xSize) + y) = input;
    return true;
}

char writeIntMatrixDirect(IntMatrix* me,int pos, int input)
{
    if(pos >= ((me->m_xSize)*(me->m_ySize)) || pos <0 || me->m_tab==NULL) return false;
    *(me->m_tab+pos)=input;
    return true;
}
char printIntMatrix(IntMatrix * me)
{
    if(me->m_xSize<=0 || me->m_ySize <=0) return false;
    int iSize=me->m_xSize * me->m_ySize;
    for(int i=0;i<iSize ;i++)
    {
        printf("%d\n",*(me->m_tab+i));
    }
    return true;
}

char readAsciInt(IntMatrix * me, FILE * file) // only use on clear martix
{
    if(file == NULL) return false;
    char line[LINE_LEN];
    int i=0;
    int iXsize;
    int iYsize;
    if(fgets(line,LINE_LEN -1,file)==NULL)
    {
        printf("\n Empty File\n");
        return false;
    } // title line
    if(!checkLine(line)) return false; // line tooo long
    if(fgets(line,LINE_LEN -1,file)==NULL)
    {
        printf("invalid file format\n");
        return false;
    }
    if(!checkLine(line)) return false; // line tooo long
    for(i=0;i<LINE_LEN;i++) if(line[i]!=' ') break;
    iYsize=atoi(line + i);
    for(;i<LINE_LEN;i++) if(line[i]==' ') break; // search for the space
    i++;// point to the next
    iXsize=atoi(line +i);
    int iPixelMaxCount=(iXsize*iYsize) ;
    if ( ! initIntMatrix(me,iXsize,iYsize)) return false ;
    int iPixelCount=0;
    if(fgets(line,LINE_LEN -1,file)==NULL) // ifgonre
    {
        printf("invalid file format2\n");
        return false;
    }
    if(!checkLine(line)) return false; // line tooo long
    while(fgets(line,LINE_LEN -1,file)!=NULL)
    {
        if(line[0]=='\n' || line[0]=='\0') continue;
        i=0;
        char fEnd=false;
        while(!fEnd)
        {
            for(;i<LINE_LEN;i++) // search for number
            {
                if(line[i]>='0' && line[i]<='9') break;
            }
            if(line[i]=='\n' ||line[i]=='\0')
            {
                fEnd=true;
                continue; // contnue the while(!fend) loop
            }
            if(!writeIntMatrixDirect(me,iPixelCount,atoi(line+i)))
            {
                printf("too many pixels\n");
                return false;
            }
            iPixelCount++;

            for(;i<LINE_LEN;i++) if(line[i]<'0' || line[i]>'9') break;
            if(line[i]=='\n' || line[i]=='\0')
            {
                fEnd=true;
            }
            if(i>=LINE_LEN) return returnWithError("Some Line is too long\n");
        }
    }//end while(fegts)
    if(iPixelCount != iPixelMaxCount)
    {
        printf("insuficent pixels given(%d|%d)\n",iPixelCount,iPixelMaxCount);
        return false;
    }
    printf("\n\n/DEBUG/  Dimensions : x:%d,y:%d\n\n",me->m_xSize,me->m_ySize);
    return true;
}

char initDoubleMatrix(DoubleMatrix * me,int x,int y)
{
    if( y <= 0 || x <= 0) return false;
    me->m_xSize=x;
    me->m_ySize=y;
    me->m_tab= malloc((sizeof (double))*x*y);
    if(me->m_tab == NULL)
    {
        me->m_xSize=0;
        me->m_ySize=0;
        return false;
    }
    return true;
}
char clearDoubleMatrix(DoubleMatrix * me)
{
    if(me->m_xSize<=0 || me->m_ySize <=0) return false;
    free(me->m_tab);
    return true;
}

double readDoubleMatrix(DoubleMatrix * me,int x,int y)
{
    if(me->m_xSize<=0 || me->m_ySize <=0 || me->m_tab==NULL || x >= me->m_xSize || y>= me->m_ySize || x < 0 || y < 0 ) return 0;
    return *(me->m_tab +(x * me->m_xSize) + y);
}

char writeDoubleMatrix(DoubleMatrix * me,int x,int y, double input)
{
    if(me->m_xSize<=0 || me->m_ySize <=0 || me->m_tab==NULL || x >= me->m_xSize || y>= me->m_ySize || x < 0 || y < 0 ) return false;
    *(me->m_tab +(x * me->m_xSize) + y) = input;
    return true;
}

char writeDoubleMatrixDirect(DoubleMatrix * me,int pos, double input)
{
    if(pos >= ((me->m_xSize)*(me->m_ySize)) || pos <0 || me->m_tab==NULL) return false;
    *(me->m_tab+pos)=input;
    return true;
}

char readAsciDouble(DoubleMatrix * me, FILE * file) // only use on clear matrix
{
    if(file==NULL)
    {
        return false;
    }
    char line[LINE_LEN];
    int iSizeOneDim;
    if(fgets(line,LINE_LEN-1,file)==NULL) return returnWithError("invalid file format\n");
    printf("Line>%s\n",line);
    if(!checkLine(line)) returnWithError("too long line\n");
    int i=0;
    for(;i<LINE_LEN;i++) if(line[i]!=' ') break;
    iSizeOneDim=atoi(line+i);
    printf("size got >%d\n",iSizeOneDim);
    int iNumbersCurrent=0;
    int iNumbersMax=iSizeOneDim*iSizeOneDim;
    if(!initDoubleMatrix(me,iSizeOneDim,iSizeOneDim)) return returnWithError("Could not create Matrix");
    while(fgets(line,LINE_LEN -1,file)!=NULL)
    {
        printf("%s\n",line);
        if(line[0]=='\n' || line[0]=='\0') continue;
        i=0;
        char fEnd=false;
        while(!fEnd)
        {
            for(;i<LINE_LEN;i++) // search for number
            {
                if((line[i]>='0' && line[i]<='9')||line[i]=='+'||line[i]=='-') break;
            }
            if(line[i]=='\n' ||line[i]=='\0')
            {
                fEnd=true;
                continue; // contnue the while(!fend) loop
            }
            if(!writeDoubleMatrixDirect(me,iNumbersCurrent,atof(line+i)))
            {
                printf("too many numbers \n");
                return false;
            }
            iNumbersCurrent++;

            for(;i<LINE_LEN;i++) if((line[i]<'0' || line[i]>'9')&& line[i]!='.' && line[i]!='+' && line[i]!='-') break;
            if(line[i]=='\n' || line[i]=='\0' )
            {
                fEnd=true;
            }
            if(i>=LINE_LEN) return returnWithError("Some Line is too long\n");
        }
    }//end while(fegts)
    if(iNumbersCurrent<iNumbersMax)
    {
        printf("insuficent numbers in filter");
        return false;
    }
    return true;

}

char printDoubleMatrix(DoubleMatrix* me)
{
    if(me->m_xSize<=0 || me->m_ySize <=0) return false;
    int iSize=me->m_xSize * me->m_ySize;
    printf("size > %d\n",iSize);
    for(int i=0;i<iSize ;i++)
    {
        printf("%f\n",*(me->m_tab+i));
    }
    return true;
}



