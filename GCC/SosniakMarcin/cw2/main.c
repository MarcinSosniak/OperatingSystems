#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myAlloc.h"
#include <time.h>
#include <sys/times.h>
#include <stdlib.h>
#define true 1
#define false 0


#define BILLION 1000000000







/*
1) stworzenie tablicy, wyszukanie elementu oraz usunięcie i dodanie zadanej liczby bloków

2) stworzenie tablicy, usunięcie i dodanie zadanej liczby bloków i

3)stworzenie tablicy naprzemienne usunięcie i dodanie zadanej liczby bloków
*/

//foo 1024 512 1 2 15

// kolejnosc argumetnow size, blocksize version, operations, 'add/delete count'
int main(int argc, char* argv[])
{

    if(argc!=6) return -1;



    struct timespec timeRealStart,timeRealEnd;
    struct tms timeProcStart,timeProcEnd;
    int size=atoi(argv[1]);
    int blocksize=atoi(argv[2]);
    int version=atoi(argv[3]);
    int operations=atoi(argv[4]);
    int addDeleteC=atoi(argv[5]);
    if(addDeleteC>size)
        addDeleteC=size;


    double realT;
    int64_t userTime;
    int64_t coteTime;


    srand(time(NULL));
    char* testBlock= malloc(size);

    for(int i=0;i<size;i++)
    {
        *(testBlock + i)= rand()%128;
    }

    if(version==1)
    {

        clock_gettime(CLOCK_REALTIME,&timeRealStart);
        times(&timeProcStart);
        {

            alloc1(size,blocksize);
        }
        times(&timeProcEnd);
        clock_gettime(CLOCK_REALTIME,&timeRealEnd);


        realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
        userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
        coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
        printf("alloc1(%d,%d) empty\nrealTime:%f ns\nuserTime:%d|ncoreTime:%d \n",size,blocksize,realT,userTime,coteTime);
        free1();
        printf("\n");

        // end of test 1
        clock_gettime(CLOCK_REALTIME,&timeRealStart);
        times(&timeProcStart);
        {

            alloc1(size,blocksize);
            for(int i=0;i<size;i++)
            {
                add1(i,testBlock);
            }
        }
        times(&timeProcEnd);
        clock_gettime(CLOCK_REALTIME,&timeRealEnd);


        realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
        userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
        coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
        printf("alloc1(%d,%d) alloc and fill\nrealTime:%f ns\nuserTime:%d|ncoreTime:%d \n",size,blocksize,realT,userTime,coteTime);
        // end of test 2

        printf("\n");



        if(operations==1)
        {

            for(int i=0; i<size;i++)
            {
                *(testBlock)=rand()%128;
                add1(i,testBlock);
            }


            int randIndex=rand()%size;
            clock_gettime(CLOCK_REALTIME,&timeRealStart);
            times(&timeProcStart);
            {
                search1(randIndex);
            }
            times(&timeProcEnd);
            clock_gettime(CLOCK_REALTIME,&timeRealEnd);


            realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
            userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
            coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
            printf("search1(%d) empty\nrealTime:%f ns\nuserTime:%d|ncoreTime:%d \n",randIndex,realT,userTime,coteTime);
            // -------------------------end of test 3.1
            printf("\n");
        }//END OPERATIONS 1



        if(operations==1 || operations==2)
        {
            clock_gettime(CLOCK_REALTIME,&timeRealStart);
            times(&timeProcStart);
            {

                for(int i=0;i<addDeleteC;i++)
                {
                    remove1(i);
                }
                for(int i=0;i<addDeleteC;i++)
                {
                    add1(i,testBlock);
                }
            }
            times(&timeProcEnd);
            clock_gettime(CLOCK_REALTIME,&timeRealEnd);


            realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
            userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
            coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
            printf("remove1(i) x %d, a potemm add1(i) x %d empty\nrealTime:%f ns\nuserTime:%d|ncoreTime:%d \n",addDeleteC,addDeleteC,realT,userTime,coteTime);
            printf("\n");
        }


        if(operations ==3)
        {
            clock_gettime(CLOCK_REALTIME,&timeRealStart);
            times(&timeProcStart);
            {

                for(int i=0;i<addDeleteC;i++)
                {
                    remove1(i);
                    add1(i,testBlock);
                }
            }
            times(&timeProcEnd);
            clock_gettime(CLOCK_REALTIME,&timeRealEnd);


            realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
            userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
            coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
            printf("remove1(i), add1(i) x %d empty\nrealTime:%f ns\nuserTime:%d|ncoreTime:%d \n",addDeleteC,realT,userTime,coteTime);
            printf("\n");
        }
        free1();
    }
    else
//////////////////////////////////////////////////////////////////////////////////VERSJA 2//////////////////////////////////////////////////////////////////////////////
    {
        clock_gettime(CLOCK_REALTIME,&timeRealStart);
        times(&timeProcStart);
        {

            alloc2(size,blocksize);
        }
        times(&timeProcEnd);
        clock_gettime(CLOCK_REALTIME,&timeRealEnd);


        realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
        userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
        coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
        printf("alloc2(%d,%d) empty\nrealTime:%f ns\nuserTime:%d|ncoreTime:%d \n",size,blocksize,realT,userTime,coteTime);
        free2();
        printf("\n");
        // end of test 1
        clock_gettime(CLOCK_REALTIME,&timeRealStart);
        times(&timeProcStart);
        {

            alloc2(size,blocksize);
            for(int i=0;i<size;i++)
            {
                add2(i,testBlock);
            }
        }
        times(&timeProcEnd);
        clock_gettime(CLOCK_REALTIME,&timeRealEnd);


        realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
        userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
        coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
        printf("alloc2(%d,%d) alloc and fill\nrealTime:%f ns\nuserTime:%d|ncoreTime:%d \n",size,blocksize,realT,userTime,coteTime);
        // end of test 2
        printf("\n");




        if(operations==1)
        {

            for(int i=0; i<size;i++)
            {
                *(testBlock)=rand()%128;
                add2(i,testBlock);
            }


            int randIndex=rand()%size;
            clock_gettime(CLOCK_REALTIME,&timeRealStart);
            times(&timeProcStart);
            {
                search2(randIndex);
            }
            times(&timeProcEnd);
            clock_gettime(CLOCK_REALTIME,&timeRealEnd);


            realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
            userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
            coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
            printf("search2(%d) empty\nrealTime:%f ns\nuserTime:%d|ncoreTime:%d \n",randIndex,realT,userTime,coteTime);
            // -------------------------end of test 3.1
            printf("\n");
        }//END OPERATIONS 1



        if(operations==1 || operations==2)
        {
            clock_gettime(CLOCK_REALTIME,&timeRealStart);
            times(&timeProcStart);
            {

                for(int i=0;i<addDeleteC;i++)
                {
                    remove2(i);
                }
                for(int i=0;i<addDeleteC;i++)
                {
                    add2(i,testBlock);
                }
            }
            times(&timeProcEnd);
            clock_gettime(CLOCK_REALTIME,&timeRealEnd);


            realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
            userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
            coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
            printf("remove2(i) x %d, a potemm add2(i) x %d empty\nrealTime:%f ns\nuserTime:%d|ncoreTime:%d \n",addDeleteC,addDeleteC,realT,userTime,coteTime);
            printf("\n");
        }


        if(operations ==3)
        {
            clock_gettime(CLOCK_REALTIME,&timeRealStart);
            times(&timeProcStart);
            {

                for(int i=0;i<addDeleteC;i++)
                {
                    remove2(i);
                    add2(i,testBlock);
                }
            }
            times(&timeProcEnd);
            clock_gettime(CLOCK_REALTIME,&timeRealEnd);


            realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
            userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
            coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
            printf("remove2(i) add2(i) x %d empty\nrealTime:%f ns\nuserTime:%d|ncoreTime:%d \n",addDeleteC,realT,userTime,coteTime);
            printf("\n");
        }
        free2();

    }

    return 0;
}








/*


    char randomblock[512]="12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345";

    clock_gettime(CLOCK_REALTIME,&timeRealStart);
    times(&timeProcStart);
{

    alloc1(size,blocksize);
}
    times(&timeProcEnd);
    clock_gettime(CLOCK_REALTIME,&timeRealEnd);




    realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
    userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
    coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
    printf("alloc1(%d,%d)\nrealTime:%f ns\nuserTime:%d|ncoreTime:%d \n",size,blocksize,realT,userTime,coteTime);



    clock_gettime(CLOCK_REALTIME,&timeRealStart);
    times(&timeProcStart);
    alloc2(size,blocksize);
    times(&timeProcEnd);
    clock_gettime(CLOCK_REALTIME,&timeRealEnd);
    realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
    userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
    coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
    printf("alloc2(%d,%d)\nrealTime:%f ns\nuserTime:%d|ncoreTime:%d \n",size,blocksize,realT,userTime,coteTime);

    free1();
    free2();



*/

































    /*

    char* hue= malloc(12);
    hue="Ala ma Kota";
    printf("%s\n\n",hue);
    alloc1(4,12);
    char* t1;

    add1(0,hue);
    add1(1,hue);
    add1(2,hue);
    add1(3,hue);

    t1=get1(0);
    printf("%s\n",t1);

    t1=get1(1);
    printf("%s\n",t1);

    t1=get1(2);
    printf("%s\n",t1);

    t1=get1(3);
    printf("%s\n",t1);

    remove1(3);
    printf("----");

    t1=get1(3);
    printf("%s\n",t1);

    free1();
    printf("%d",(int)add1(0,hue));
    printf("\n\n\n============================================================\n\n\n");

    alloc1(4,12);
    char* hue2=malloc(12);
    hue2="Ala ma kota";
    char* zeroField=malloc(12);
    for(int i=0;i<12;i++)
    {
        *(zeroField+i)='\0';
    }
    add1(0,hue);
    add1(1,hue2);
    add1(2,zeroField);
    add1(3,zeroField);

    t1=get1(0);
    printf("0>%s\n",t1);

    t1=get1(1);
    printf("1>%s\n",t1);

    t1=get1(2);
    printf("2>%s\n",t1);

    t1=get1(3);
    printf("3>%s\n",t1);

    printf("%d",search1(1));

    printf("\n\n\n============================================================\n\n\n");
    printf("\n\n\n==========================PART2=============================\n\n\n");
    alloc2(4,12);


    char* t2;

    add2(0,hue);
    add2(1,hue);
    add2(2,hue);
    add2(3,hue);
    printf("\nhue\n");
    t2=get2(0);
    printf("0>%s\n",t2);

    t2=get2(1);
    printf("1>%s\n",t2);

    t2=get2(2);
    printf("2>%s\n",t2);

    t2=get2(3);
    printf("3>%s\n",t2);

    remove2(3);
    t2=get2(3);
    printf("3post dest>%d",t2);

    free2();


    printf("\n\n\n============================================================\n\n\n");
    printf("\n\n\n==========================PART3=============================\n\n\n");
    alloc2(4,12);

    add2(0,hue);
    add2(1,hue2);
    add2(2,zeroField);
    add2(3,zeroField);

    t2=get2(0);
    printf("0>%s\n",t2);

    t2=get2(1);
    printf("1>%s\n",t2);

    t2=get2(2);
    printf("2>%s\n",t2);

    t2=get2(3);
    printf("3>%s\n",t2);

    printf("%d",search2(0));
    free2();
    //free(hue);
    */
