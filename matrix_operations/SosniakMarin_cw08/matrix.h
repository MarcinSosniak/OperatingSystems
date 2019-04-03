#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED
#include "stdio.h"
#include "stdlib.h"

#ifndef true
#define true 1
#endif // true

#ifndef false
#define false 0
#endif // false


#define LINE_LEN 256


char returnWithError(const char*);

typedef struct
{
    int m_xSize;
    int m_ySize;
    int * m_tab;
}IntMatrix;

// DO NOT USE IF WANT TO DO FILE READ
char initIntMatrix(IntMatrix * me,int x,int y);
//
char clearIntMatrix(IntMatrix * me);
int readIntMatrix(IntMatrix * me,int x,int y);
char writeIntMatrix(IntMatrix * me,int x,int y, int input);
char readAsciInt(IntMatrix * me, FILE * file); // only use on clear martix
char printIntMatrix(IntMatrix * me);

typedef struct
{
    int m_xSize;
    int m_ySize;
    double * m_tab;
}DoubleMatrix;

// DO NOT USE IF WANT TO DO FILE READ
char initDoubleMatrix(DoubleMatrix * me,int x,int y);
//
char clearDoubleMatrix(DoubleMatrix * me);
double readDoubleMatrix(DoubleMatrix * me,int x,int y);
char writeDoubleMatrix(DoubleMatrix * me,int x,int y, double input);
char readAsciDouble(DoubleMatrix * me, FILE * file); // only use on clear matrix
char printDoubleMatrix(DoubleMatrix* me);





#endif // MATRIX_H_INCLUDED
