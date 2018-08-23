#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "line_parser.h"
#define TRUE 1
#define LINE_MAX_SIZE 2048
#define STDIN 0
#define STDOUT 1

cmd_line* parsedCmdLine;
int pid;

void execute(cmd_line* line){
    if (execvp(line->arguments[0],line->arguments) == -1){
        perror(line->arguments[0]);
        _exit(EXIT_FAILURE);
    }
}

void childProccess(){
    if(parsedCmdLine->input_redirect != NULL){
        close(STDIN);
        fopen(parsedCmdLine->input_redirect , "r");
    }
    if(parsedCmdLine->output_redirect != NULL){
        close(STDOUT);
        fopen(parsedCmdLine->output_redirect , "w");
    }
    execute(parsedCmdLine); 
}

void parentProcess(){
    if(parsedCmdLine->blocking == 1){
        if(waitpid(pid,NULL,0) == -1){
            perror("waitpid");
            free_cmd_lines(parsedCmdLine);
            exit(EXIT_FAILURE);
        }
    }
}

int main (int argc , char* argv[]){
    char cwd[PATH_MAX];
    char inputLine[LINE_MAX_SIZE];
    int pipefd[2];
    
    while(TRUE){
        getcwd(cwd,PATH_MAX);
        printf("%s$:",cwd);
        fgets(inputLine,LINE_MAX_SIZE,stdin);
        
        if (strcmp("quit\n",inputLine) == 0){
            break;
        }
        if (strcmp("\n",inputLine) == 0)
            continue;
        
        parsedCmdLine = parse_cmd_lines(inputLine);
        
        if(parsedCmdLine->next != NULL){
            
            if(pipe(pipefd) == -1){
                perror("pipe");
                exit(EXIT_FAILURE);
            }
            
            int pid1,pid2;
            
            switch(pid1 = fork()){
                default:
                    close(pipefd[1]);
                    break;
                    
                case 0:
                    close(STDOUT);
                    dup(pipefd[1]);
                    close(pipefd[1]);
                    childProccess();
                    break;
            
                case -1:
                    perror("fork");
                    exit(EXIT_FAILURE);       
            }
            
            switch (pid2 = fork()){
                default:
                    close(pipefd[0]);
                    break;
                case 0:
                    close(STDIN);
                    dup(pipefd[0]);
                    close(pipefd[0]);
                    parsedCmdLine = parsedCmdLine->next;
                    childProccess();
                    break;
            
                case -1:
                    perror("fork");
                    exit(EXIT_FAILURE);       
            }
    
            waitpid(pid1,NULL,0);
            waitpid(pid2,NULL,0);
        } // if
        
        else{
            switch (pid = fork()){
                default: //this is the parent proccess
                    parentProcess();
                    break;
                    
                case 0: //this is the child proccess
                    childProccess();
                    break;
                    
                case -1:
                    perror("fork");
                    free_cmd_lines(parsedCmdLine);
                    exit(EXIT_FAILURE);
            }
        }
        
        free_cmd_lines(parsedCmdLine);
    }//while
    return 0;
}