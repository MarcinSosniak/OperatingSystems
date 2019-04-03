#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <string.h>
#define true 1
#define false 0

//#define DBG_1 1
//#define SHOW 1



#define PATH_MAX_LEN 200
#define MAX_LINE_PIPES 10
#define MAX_ARGS 10
#define MAX_LINES 15

#ifdef SHOW
void dbgPrint(char* arg[])
{
    for(int i=0;i<(MAX_LINE_PIPES*(MAX_ARGS+2));i++)
    {
        if(arg[i]==NULL)
        {
            printf("%03d:Size=%03d>NULL \n",i,0);
        }
        else
        {
            printf("%03d:Size=%03d>%s \n",i,strlen(arg[i]),arg[i]);
        }
    }
}
#endif // SHOW













char execute(char* path)
{
    FILE* inputFile=fopen(path,"r");
    if(inputFile==NULL) return false;

    char sFile[MAX_LINES*PATH_MAX_LEN];
    long long int iTotalChars;
    long long int iChCr=0;
    iTotalChars=fread(sFile,1,MAX_LINES*PATH_MAX_LEN-2,inputFile);
    fclose(inputFile);
    int iPipesNr=0;
    int status=0;
    int iProccesArgsID[MAX_LINE_PIPES];
    char* sCurrentLine=sFile;
    char* sNextLine=sCurrentLine;
    int iCharsRead;
    char* sArgs[MAX_LINE_PIPES*(MAX_ARGS+2)]; // table for all arguments for all execs in line;


    int iPID;
    int i;
    int iArgsID;

    char fEOF=false;
    int iPIDs[MAX_LINE_PIPES+1];
    #ifdef DBG_1
    int dbgC=0;
    #endif // DBG_1

    while(!fEOF)
    {
        sCurrentLine=sNextLine;
        #ifdef DBG_1
        printf("\n>%d",dbgC);
        dbgC++;
        #endif

        //fgets(sCurrentLine,PATH_MAX_LEN,inputFile);

        #ifdef DBG_1
        printf(">%s\n",sCurrentLine);
        continue;
        #endif
        if(sCurrentLine[0]=='\0') return true;
        iCharsRead=0;




        for(iCharsRead =0; iCharsRead < PATH_MAX_LEN && iChCr<=iTotalChars;iCharsRead++)
        {
            iChCr++;
            if(*(sCurrentLine+ iCharsRead)=='\n' || *(sCurrentLine+ iCharsRead)=='\0' )
            {
                *(sCurrentLine+ iCharsRead)='\0';
                sNextLine=(sCurrentLine+iCharsRead +1);
                break;
            }
        }

        if(sCurrentLine[0]=='#')
        {
            sNextLine=sCurrentLine+iCharsRead+1;
            continue;// skip li e
        }
        if(iChCr>=iTotalChars) fEOF=true;
        if(iCharsRead==PATH_MAX_LEN)
        {
            return false;
        }
        if(iCharsRead==0 ||iCharsRead==1 ) continue; // if empty line
        printf("%s\n", sCurrentLine);

        for(i=0;i<iCharsRead;i++)
        {
            if(sCurrentLine[i]==' ')
                sCurrentLine[i]='\0';
        }



        for(i=0;i<MAX_LINE_PIPES;i++)
        {
            iProccesArgsID[i]=0;
        }


        // hue

        char fAnyArgs=false; //   exclucsion for "   | ps aux"
        iPipesNr=0;
        iArgsID=0;
        i=0;


        // Parsing line --------------------------------------------
        for(int i=0;i<iCharsRead;i++)
        {
            for(;i<iCharsRead && sCurrentLine[i]=='\0';i++)
                ;
            if(sCurrentLine[i]=='|')
            {
                if(!fAnyArgs) return false;
                iPipesNr++;
                sArgs[iArgsID]=NULL;
                iArgsID++;
                if(iArgsID >= (MAX_LINE_PIPES*(MAX_ARGS+2) )|| iPipesNr >= MAX_LINE_PIPES) return false; // Error over-flow
                iProccesArgsID[iPipesNr]=iArgsID;
                continue;
            }
            if(i==iCharsRead) break;
            sArgs[iArgsID]=&sCurrentLine[i];
            fAnyArgs=true;
            iArgsID++;
            if(iArgsID>=(MAX_LINE_PIPES*(MAX_ARGS+2))) return false;

            for(;i<iCharsRead && sCurrentLine[i]!='\0';i++);
                ;
        }
        sArgs[iArgsID]=NULL; // terminate last Args tab for exec call.

        #ifdef SHOW
        dbgPrint(sArgs);
        printf("iPipesNr:%d\n",iPipesNr);
        for(int k =0;k<=iPipesNr;k++)
            printf("iPipesMaxNumbers[%d]:%d\n",k,iProccesArgsID[k]);
        //return true;
        #endif //SHOW
        int iPipesNrCpy=iPipesNr;
        if(iPipesNr<=0) iPipesNrCpy=1; // to avoid crashes on table creation
        int pipesDescTab [iPipesNrCpy][2];
        for(i=0;i<iPipesNr;i++)
        {
            pipe(&pipesDescTab[i][0]);
        }

        for(i=0;i<(iPipesNr+1);i++ )
        {
            iPID=fork();
            if(iPID == 0)
                break;
            iPIDs[i]=iPID;
        }

        int iCurrentExecNr=i;

        if(iPID==0)
        {
            for(i=0;i<iPipesNr;i++)
            {
                if(i!=(iCurrentExecNr) && i!=(iCurrentExecNr-1))
                {
                    close(pipesDescTab[i][0]);
                    close(pipesDescTab[i][1]);
                }

            }
            if(iPipesNr!=0)
            {
                if(iCurrentExecNr==iPipesNr)
                {
                    close(pipesDescTab[iPipesNr-1][1]);
                    dup2(pipesDescTab[iPipesNr-1][0],STDIN_FILENO);
                    }
                else    if(iCurrentExecNr==0)
                {
                    close(pipesDescTab[0][0]);
                    dup2(pipesDescTab[0][1],STDOUT_FILENO);

                }
                else
                {
                    close(pipesDescTab[iCurrentExecNr][0]);
                    dup2(pipesDescTab[iCurrentExecNr][1],STDOUT_FILENO);
                    close(pipesDescTab[iCurrentExecNr-1][1]);
                    dup2(pipesDescTab[iCurrentExecNr-1][0],STDIN_FILENO);
                }
            }

            // CALL desisred program;

            if(*sArgs[iProccesArgsID[iCurrentExecNr]]=='.' || *sArgs[iProccesArgsID[iCurrentExecNr]]=='/')
            {
                execv(sArgs[iProccesArgsID[iCurrentExecNr]],&sArgs[iProccesArgsID[iCurrentExecNr]]);
            }
            else
            {
                execvp(sArgs[iProccesArgsID[iCurrentExecNr]],&sArgs[iProccesArgsID[iCurrentExecNr]]);
            }

            exit(-1);// if soemthing went wrong die;
        }




        //close mother pipes;
        for(int k=0;k<iPipesNr;k++)
        {
            close(pipesDescTab[k][0]);
            close(pipesDescTab[k][1]);
        }


        for(i=0;i<(iPipesNr+ 1 ) && iPID!=0; i++)
        {
            waitpid(iPIDs[i],&status,0);
            if(status!=0)
            {
                return false;
            }
        }
        // END MOTHER
        #ifdef SHOW
        printf("\n\n ENDING WHILE \n\n");
        #endif // SHOW
    }// end while

    return true;
}



int main(int args, char* argv[])
{
    char* cb1="/sysOp/lab5/cw1/script";
    char* path=cb1;
    if(args==2)
    {
        path=argv[1];
    }
    if(false==execute(path))
        printf("\n-----An fatal error has occured-----\n");

    return 0;
}
