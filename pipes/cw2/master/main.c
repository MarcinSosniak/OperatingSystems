#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define true 1
#define false 0

char creteFIFOandListen(char* path)
{
    if(!mkfifo(path,S_IRUSR  | S_IWUSR | S_IROTH | S_IWOTH | S_IRUSR))
        return false;
    char chBuff[100];
    int fd;
    int iBytesRead;
    fd=open(path,O_RDONLY);

        if(fd<0)
    {
        printf("\n\nCannot Open FIFO\n\n");
        return false;
    }
    while( true )
    {
        //printf("\nHUE\n");
        iBytesRead=read(fd,chBuff,60);
        if(iBytesRead>0)
        {
            printf("%s",chBuff);
            fflush(stdout);
        }
        else
        {
            break;
        }
    }
    close(fd);
    return true;
}






int main(int args, char* argv[])
{

    char * path;
    char pathBuff[]="/tmp/potok";
    if(args == 2)
        path=argv[1];
    else
        path=pathBuff;
    creteFIFOandListen(path);
    printf("Hello world!\n");

    return 0;

}
