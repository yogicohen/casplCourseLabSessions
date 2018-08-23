#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "line_parser.h"
#define STDIN 0
#define STDOUT 1

void execute(cmd_line* line){
    if (execvp(line->arguments[0],line->arguments) == -1){
        perror(line->arguments[0]);
        _exit(EXIT_FAILURE);
    }
}

int main (int argc , char* argv[]){
    int pipefd[2];
    int pid1,pid2;
    
    cmd_line* ls = parse_cmd_lines("ls -l");
    cmd_line* tail = parse_cmd_lines("tail -n 2");
    
    if(pipe(pipefd) == -1){
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    switch (pid1 = fork()){
        default://parent proccess
            close(pipefd[1]);
            break;
        case 0://child proccess
            close(STDOUT);
            dup(pipefd[1]);
            close(pipefd[1]);
            execute(ls);
            break;
            
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);       
    }
    
    switch (pid2 = fork()){
        default://parent proccess
            close(pipefd[0]);
            break;
        case 0://child proccess
            close(STDIN);
            dup(pipefd[0]);
            close(pipefd[0]);
            execute(tail);
            break;
            
        case -1:
            perror("fork");
            exit(EXIT_FAILURE);       
    }
    
    waitpid(pid1,NULL,0);
    waitpid(pid2,NULL,0);
    
    free_cmd_lines(ls);
    free_cmd_lines(tail);
    return 0;
}