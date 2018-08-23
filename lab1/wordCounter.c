#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


int main(int argc, char **argv){
    FILE * output=stdout;
    FILE * input;
    int wFlag = 0;
    int cFlag = 0;
    int lFlag = 0;
    int nFlag = 0;
    int iFlag = 0;
    int i;
    
    
    for(i=1; i<argc; i++){
        if(strcmp(argv[i],"-w")==0)
            wFlag = 1;
        else if(strcmp(argv[i],"-c")==0)
            cFlag = 1;
        else if(strcmp(argv[i],"-l")==0)
            lFlag = 1;
        else if(strcmp(argv[i],"-n")==0)
            nFlag = 1;
        else if(strcmp(argv[i],"-i")==0){
            iFlag = 1;
            input = fopen(argv[i+1],"r");
        }
    }
    
    if (wFlag == 0 && cFlag == 0 && lFlag ==0 && nFlag==0)
        wFlag = 1;
    
    char line[LINE_MAX];
    int c=0;
    i=0;
    if (iFlag==0){
        while ((c = fgetc(stdin)) != EOF)
            line[i++]=(char)c;
    }
    else {
        while ((c = fgetc(input)) != EOF)
            line[i++]=(char)c;
    }
    
    int lineLength = strlen(line);
    int lastCharWasSpace = 0;
    int wordCounter, charCounter = 0;
    int longestWordCounter = 0;
    int tmpCounter = 0;
    int linesCounter = 0;
    
    
    for (i=0 ; i<lineLength ; i++){
        if(line[i]<= 0x20){
            if(lastCharWasSpace)
                continue;
            else{
                wordCounter++;
                lastCharWasSpace=1;
                if(tmpCounter>longestWordCounter)
                    longestWordCounter=tmpCounter;
                tmpCounter=0;
            }
        }
        else{
            charCounter++;
            tmpCounter++;
            lastCharWasSpace=0;
        }
        if (line[i] == '\n')
            linesCounter++;
    }
    
    if (wFlag == 1)
        fprintf(output,"%d\n",wordCounter);
    if (cFlag == 1)
        fprintf(output,"%d\n",charCounter);
    if (lFlag == 1)
        fprintf(output,"%d\n",longestWordCounter);
    if (nFlag == 1)
        fprintf(output,"%d\n",linesCounter);
    if(output!=stdout)
        fclose(output);
    
    return 0;
}


