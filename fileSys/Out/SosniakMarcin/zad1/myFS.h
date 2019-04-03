#ifndef MYFS_H_INCLUDED
#define MYFS_H_INCLUDED

char generate(char * path, int size, int blockSize);
char copyLib(char * pathSRC,char * pathTRGT, int size, int blockSize);
char copySys(char * pathSRC,char * pathTRGT, int size, int blockSize);
char sortSys(char* pathSRC, int iSize, int iBlockSize);
char sortLib(char* pathSRC, int iSize, int iBlockSize);
#endif // MYFS_H_INCLUDED
