#include <stdio.h>
#include <stdlib.h>

void PowerOverwhelming(int level)
{
    char* hue= malloc(10*1024*1024);
    int ha=10*( level+1);
    printf("zuzyte %d megaRamu \n",ha);
    PowerOverwhelming(level+1);
    free(hue);// /// huehuehhuuhhuuhehuheuhehueuheuhehueuheuheuhe
}


int main()
{
    PowerOverwhelming(0);
    return 0;
}
