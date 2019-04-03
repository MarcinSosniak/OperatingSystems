#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <ftw.h>
#include <time.h>
#include <string.h>



#define FTW_PHYS_TOCOS_NIEDZIALA 1

#define true 1
#define false 1


struct timeBeg
{
    time_t base_time;
    char comparing;
};

struct timeBeg bt = {0,5};;




static int fn(const char* fpath,const struct stat *sb,int typeFlag)
{
    //printf("i have been called\n");
    if(typeFlag==FTW_F)
    {
        if( (bt.comparing==1 && sb->st_mtime > bt.base_time) || (bt.comparing==0 && sb->st_mtime == bt.base_time) || (bt.comparing== -1 && sb->st_mtime < bt.base_time))
        {
            char buf[10];
            char cbTime[30];
            const char chars[] = "rwxrwxrwx";
            for (size_t i = 0; i < 9; i++)
            {
                buf[i] = (sb->st_mode & (1 << (8-i))) ? chars[i] : '-';
            }
            buf[9] = '\0';
            printf("Path:> %s\n",fpath);
            printf("Size: %d",sb->st_size);
            printf(" Permissions>%s\n",buf);
            strftime(cbTime,30,"%Y-%m-%d %H:%M:%S",localtime(&sb->st_mtime));
            printf("Last modification time:>%s\n\n",cbTime);
        }
    }
    return 0;
}


char traverseAndWrite(char* basePath)
{

    DIR* dir;
    struct dirent* fData;
    dir=opendir(basePath);
    struct stat statB;
    if(dir==NULL) return false;
    fData=readdir(dir);
    while(fData!=NULL)
    {
       // printf("!");
        if(0==strcmp(fData->d_name,".")  ) {fData=readdir(dir);continue;}
        if(0==strcmp(fData->d_name,"..")) {fData=readdir(dir);continue;}
        char* path=malloc(strlen(basePath) +  strlen(fData->d_name) + 10);
        *path='\0';
        strcat(path,basePath);
        strcat(path,"/");
        strcat(path,fData->d_name);


        //int dbg=0;
        //dbg=
        stat(path,&statB);
        //Sprintf("\nfData->d_name=%s\n Czy folder?%d",fData->d_name,S_ISDIR(statB.st_mode) );
        if(S_ISDIR(statB.st_mode)) // if is dir go go power reccurence;
        {
            traverseAndWrite(path);
        }

        if(S_ISREG(statB.st_mode)) //is regural file
        {
            if( (bt.comparing==1 && statB.st_mtime > bt.base_time) || (bt.comparing==0 && statB.st_mtime == bt.base_time) || (bt.comparing== -1 && statB.st_mtime < bt.base_time))
            {
                char buf[10];
                char cbTime[30];
                const char chars[] = "rwxrwxrwx";
                for (size_t i = 0; i < 9; i++)
                {
                    buf[i] = (statB.st_mode & (1 << (8-i))) ? chars[i] : '-';
                }
                buf[9] = '\0';
                printf("Path:> %s\n",path);
                printf("Size: %d",statB.st_size);
                printf(" Permissions>%s",buf);
                strftime(cbTime,30,"%Y-%m-%d %H:%M:%S",localtime(&statB.st_mtime));
                printf("Last modification time>%s\n\n",cbTime);
            }
        }
        fData=readdir(dir);
    }
    return true;
}










int main(int args, char* argv[])
{
    char* pathBase;
    char cbTest1[100];
    char cbTest2[100];
    char* pathTest=NULL;
    struct stat buf;
    if(args == 3)
    {
        if(*argv[2]=='<') bt.comparing=-1;
        if(*argv[2]=='=') bt.comparing=0;
        if(*argv[2]=='>') bt.comparing=1;
        pathBase=argv[1];
        pathTest=argv[3];
        stat(pathTest,&buf);
        bt.base_time=buf.st_mtime;
    }
    else
    {
        bt.base_time=time(NULL);
        bt.comparing=-1;
        const char path[]="/sysOp";
        pathBase=path;
    }
    //printf("Hello world!\n");
    //const char path[]="/sysOp";

    printf("Traverse nftw :\n\n\n\n\n");
    nftw(pathBase,fn,1000,FTW_PHYS_TOCOS_NIEDZIALA);
    printf("\n\n\n\nTraverse system call \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    traverseAndWrite(pathBase);
    printf("\n");

    return 0;
}
