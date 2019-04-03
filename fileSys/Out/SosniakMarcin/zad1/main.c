#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/times.h>
#include "myFS.h"
#include "string.h"


#define BILLION 1000000000

int main(int argc, char* argv[])
{
    srand(time(NULL));
    struct timespec timeRealStart,timeRealEnd;
    struct tms timeProcStart,timeProcEnd;
    double realT;
    int64_t userTime;
    int64_t coteTime;



    if(argc==5)
    {
        generate(argv[2],atoi(argv[3]),atoi(argv[4]));
        return 0;
    }
    if(argc==6)
    {
        if(*argv[5]=='s')
        {
            clock_gettime(CLOCK_REALTIME,&timeRealStart);
            times(&timeProcStart);

            sortSys(argv[2],atoi(argv[3]),atoi(argv[4]));

            times(&timeProcEnd);
            clock_gettime(CLOCK_REALTIME,&timeRealEnd);
            realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
            userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
            coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
            printf("\nsortSys(%s,%d,%d)\n RealT:%f     ,userT:%d,coreT:%d\n",argv[2],atoi(argv[3]),atoi(argv[4]),realT,userTime,coteTime);
        }
        else
        {
            clock_gettime(CLOCK_REALTIME,&timeRealStart);
            times(&timeProcStart);

            sortLib(argv[2],atoi(argv[3]),atoi(argv[4]));

            times(&timeProcEnd);
            clock_gettime(CLOCK_REALTIME,&timeRealEnd);
            realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
            userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
            coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
            printf("\nsortLib(%s,%d,%d)\n RealT:%f     ,userT:%d,coreT:%d\n",argv[2],atoi(argv[3]),atoi(argv[4]),realT,userTime,coteTime);
        }
        return 0;
    }
    if(argc==7)
    {
        if(*argv[6]=='s')
        {

            clock_gettime(CLOCK_REALTIME,&timeRealStart);
            times(&timeProcStart);

            copySys(argv[2],argv[3],atoi(argv[4]),atoi(argv[5]));

            times(&timeProcEnd);
            clock_gettime(CLOCK_REALTIME,&timeRealEnd);
            realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
            userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
            coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
            printf("\ncopySys(%s,%s,%d,%d)\n RealT:%f     ,userT:%d,coreT:%d\n",argv[2],argv[3],atoi(argv[4]),atoi(argv[5]),realT,userTime,coteTime);
        }
        else
        {

            clock_gettime(CLOCK_REALTIME,&timeRealStart);
            times(&timeProcStart);

            copyLib(argv[2],argv[3],atoi(argv[4]),atoi(argv[5]));

            times(&timeProcEnd);
            clock_gettime(CLOCK_REALTIME,&timeRealEnd);
            realT=(timeRealEnd.tv_sec - timeRealStart.tv_sec )*BILLION+ (timeRealEnd.tv_nsec - timeRealStart.tv_nsec);
            userTime=timeProcEnd.tms_utime - timeProcStart.tms_utime;
            coteTime=timeProcEnd.tms_stime - timeProcStart.tms_stime;
            printf("\ncopySys(%s,%s,%d,%d)\n RealT:%f     ,userT:%d,coreT:%d\n",argv[2],argv[3],atoi(argv[4]),atoi(argv[5]),realT,userTime,coteTime);
        }
        return 0;

    }



    /*
    printf("Hello world!\n");
    generate("data",10,128);
    printf(">%d\n",copyLib("data","dataL",10,128));
    copySys("dataT","dataT2",8,1);
    copyLib("dataT","dataT3",8,1);
    //printf(">%d\n",sortSys("dataS",10,128));
    printf(">%d\n",sortSys("dataT2",7,1));
    printf(">%d\n",sortLib("dataT3",7,1));
    */
    return 0;
}
