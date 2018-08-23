#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#define TRUE 1
#define buffSize 100

int size;
char filename[buffSize];

void SetUnitSize(){
    int input;
    char userInputBuff[LINE_MAX];
    fgets(userInputBuff,LINE_MAX,stdin);
    sscanf(userInputBuff,"%d", &input);
    
    if(input == 1 || input == 2 || input == 4){
        size = input;
        return;
    }
    else {
        printf("%s" , "Not suitable unit size\n");
        return;
    }
}

void SetFileName(){
    char filenameInput[LINE_MAX];
    fgets(filenameInput,LINE_MAX,stdin);
    sscanf(filenameInput,"%s", filename);
    return;
}

void FileDisplay(){
    if(filename[0] == '\0'){
        printf("%s", "filename is NULL\n");
        return;
    }
    FILE* f;
    if((f = fopen(filename,"r")) == NULL){
        printf("%s", "fopen has failed\n");
        return;
    }
    
    int location;
    int length;
    char input[LINE_MAX];
    unsigned char* readBuff;
    unsigned short* shortCastedBuff;
    unsigned int* uintCastedBuff;
    
    printf("%s", "Please enter <location> <length>\n");
    
    
    fgets(input,LINE_MAX,stdin);
    sscanf(input,"%x %d", &location, &length);
    
    readBuff = calloc(length,size);
    
    fseek(f, location ,SEEK_SET);
    fread(readBuff,size,length,f);
    fclose(f);
    
    int i;
    
    switch (size){ // considering the size units, need to cast the buffered array
        case 1:
            printf("%s", "Hexadecimal Representation:\n");
            for (i=0 ; i<length-1 ; ++i)
                printf("%x ",readBuff[i]);
            printf("%x\n",readBuff[i]);
            
            printf("%s", "Decimal Representation:\n");
            for (i=0 ; i<length-1 ; ++i)
                printf("%d ",readBuff[i]);
            printf("%d\n",readBuff[i]);
            
            break;
            
        case 2:
            shortCastedBuff = (unsigned short*)readBuff;
            
            printf("%s", "Hexadecimal Representation:\n");
            for (i=0 ; i<length-1 ; ++i)
                printf("%x ",shortCastedBuff[i]);
            printf("%x\n",shortCastedBuff[i]);
            
            printf("%s", "Decimal Representation:\n");
            for (i=0 ; i<length-1 ; ++i)
                printf("%d ",shortCastedBuff[i]);
            printf("%d\n",shortCastedBuff[i]);
            break;
            
        case 4:
            uintCastedBuff = (unsigned int*)readBuff;
            
            printf("%s", "Hexadecimal Representation:\n");
            for (i=0 ; i<length-1 ; ++i)
                printf("%x ",uintCastedBuff[i]);
            printf("%x\n",uintCastedBuff[i]);
            
            printf("%s", "Decimal Representation:\n");
            for (i=0 ; i<length-1 ; ++i)
                printf("%d ",uintCastedBuff[i]);
            printf("%d\n",uintCastedBuff[i]);
            break;
            
        default: 
            break;
    }
    
    free(readBuff);
    return;
}

void FileModify(){
    if(filename[0] == '\0'){
        printf("%s", "filename is NULL\n");
        return;
    }
    FILE* f;
    if((f = fopen(filename,"r+")) == NULL){
        printf("%s", "fopen has failed\n");
        return;
    }
    
    int location;
    int val;
    char input[LINE_MAX];
    
    printf("%s", "Please enter <location> <val>\n");
    
    fgets(input,LINE_MAX,stdin);
    sscanf(input,"%x %x", &location, &val);
    
    fseek(f, location ,SEEK_SET);
    fwrite(&val, size, size, f);
    fclose(f);
    return;
}

void CopyFromFile(){
    FILE* dst_file;
    FILE* src_file;
    char input[LINE_MAX];
    char src_file_name[buffSize];
    int src_offset, dst_offset, length;
    unsigned char* readBuff;

    if(filename[0] == '\0'){
        printf("%s", "filename is NULL\n");
        return;
    }
    
    if((dst_file = fopen(filename,"r+")) == NULL){
        printf("%s", "fopen has failed\n");
        return;
    }

    printf("%s","Please enter <src_file> <src_offset> <dst_offset> <length>\n" );
    fgets(input,LINE_MAX,stdin);
    sscanf(input,"%s %x %x %d", src_file_name, &src_offset, &dst_offset, &length);

    if((src_file = fopen(src_file_name,"r")) == NULL){
        printf("%s", "fopen has failed\n");
        return;
    }

    readBuff = calloc(length,size);
    
    fseek(src_file, src_offset ,SEEK_SET);
    fread(readBuff,size,length,src_file);

    fseek(dst_file, dst_offset ,SEEK_SET);
    fwrite(readBuff, size, size, dst_file);

    fprintf(stdout,"Copied %d bytes FROM %s at %X TO %s at %X\n",length,src_file_name,src_offset,filename,dst_offset);

    fclose(dst_file);
    fclose(src_file);
    free(readBuff);
    return;
}

void Quit(){
    exit(0);
}

int main(int argc, char **argv) {
    size = 1;
    int i;
    int UserChoice;
    char input[LINE_MAX];
    void (*functionArr[6])();
    
    functionArr[0] = &SetFileName;
    functionArr[1] = &SetUnitSize;
    functionArr[2] = &FileDisplay;
    functionArr[3] = &FileModify;
    functionArr[4] = &CopyFromFile;
    functionArr[5] = &Quit;
    
    const char* UserMenu[7];
    UserMenu[0] = "Choose action:\n";
    UserMenu[1] = "1-Set File Name\n";
    UserMenu[2] = "2-Set Unit Size\n";
    UserMenu[3] = "3-File Display\n";
    UserMenu[4] = "4-File Modify\n";
    UserMenu[5] = "5-Copy From File\n";
    UserMenu[6] = "6-Quit\n";
    
    while(TRUE){
        for (i=0 ; i<7 ; ++i)
            printf("%s",UserMenu[i]);
        fgets(input,LINE_MAX,stdin);
        sscanf(input,"%d", &UserChoice);
        functionArr[UserChoice-1]();
    }
    
    return 0;
}