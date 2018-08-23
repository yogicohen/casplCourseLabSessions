#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "line_parser.h"
#include "common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#define TRUE 1
#define MAX_LENGTH 2048
#define CONN_FAIL_NOT_IDLE -2
#define BYE_FAIL_NOT_CONNECTED -2
#define FAIL -1
#define RETURN_NORMAL 0
#define PORT_NUMBER "2018"

client_state state = {"nil",IDLE,NULL,-1};
int d_flag = 0;

void debug(char* msg){
    if(d_flag)
        fprintf(stderr, "%s|Log: %s \n", state.server_addr, msg);
    return;
}

void reset_state(){
    state.server_addr = "nil";
    state.sock_fd = -1;
    state.conn_state = IDLE;
    state.client_id = NULL;
    return;
}

int conn(cmd_line* parsedCmdLine){
    if(state.conn_state != IDLE){
        printf("connection failed, conn_state doesn't set to IDLE\n");
        return CONN_FAIL_NOT_IDLE;
    }
    struct addrinfo hints, *result;
    int sockfd;
    
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    
    getaddrinfo(parsedCmdLine->arguments[1],PORT_NUMBER, &hints, &result);
    
    sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    
    if(connect(sockfd, result->ai_addr, result->ai_addrlen) == 0){
        char* msgToSend = "hello";
        int msgLen, bytesSent, bytesReceived;
        msgLen = strlen(msgToSend);
        bytesSent = send(sockfd, msgToSend, msgLen, 0);

        if(bytesSent != -1){
            state.conn_state = CONNECTING;
            char recBuff[MAX_LENGTH];
            
            memset(recBuff, 0, sizeof(recBuff));
            bytesReceived = recv(sockfd, recBuff, MAX_LENGTH , 0);
            recBuff[bytesReceived] = 0;
            
            debug(recBuff);
            
            cmd_line* recievedParsedCmdLine = parse_cmd_lines(recBuff);
            
            if(strcmp(recievedParsedCmdLine->arguments[0], "hello")==0){
                state.server_addr = parsedCmdLine->arguments[1];
                state.sock_fd = sockfd;
                state.conn_state = CONNECTED;
                state.client_id = recievedParsedCmdLine->arguments[1];
            }
            else if(strcmp(recievedParsedCmdLine->arguments[0], "nok")==0){
                fprintf(stderr, "Server Error: %s\n", recievedParsedCmdLine->arguments[1]);
                reset_state();
            }
            else{
                perror("Server response unexpected");
                free_cmd_lines(recievedParsedCmdLine);
                return FAIL;
            }//unexpected response
            
            free_cmd_lines(recievedParsedCmdLine);
        }//bytesSent if
        
        else{
            perror("SEND FAILED");
            return FAIL;
        }//bytesSent else
        
    }//connect if
    
    else{
        perror("CONNECT FAILED");
        return FAIL;
    }//connect else
    
    return RETURN_NORMAL;
}

int bye(){
    if(state.conn_state == CONNECTED){
        if(state.sock_fd > 0){
            char* msgToSend = "bye";
            if (send(state.sock_fd, msgToSend , strlen(msgToSend), 0) == -1)
                perror("send failed");
            close(state.sock_fd);
        }
        reset_state();
    }
    else{
        printf("can't disconnect cause you are NOT CONNECTED!\n");
        return BYE_FAIL_NOT_CONNECTED;
    }
    return RETURN_NORMAL;
}

int ls(){
    if(state.conn_state == CONNECTED){
        int bytesReceived;
        char answer[3];
        char recievedBuffLs[MAX_LENGTH];
        memset(answer, 0 ,sizeof(answer));
        memset(recievedBuffLs, 0 ,sizeof(recievedBuffLs));
        recievedBuffLs[MAX_LENGTH] = 0;
        char* msgToSend = "ls";
        if(send(state.sock_fd, msgToSend, strlen(msgToSend), 0) == -1){
            perror("send failed");
            return FAIL;
        }
        bytesReceived = recv(state.sock_fd,answer,3,0);
        
        if(bytesReceived == -1){
            perror("recv faild");
            return FAIL;
        }

        if(strcmp(answer,"ok")==0){
            bytesReceived = recv(state.sock_fd,recievedBuffLs,MAX_LENGTH,0 );
            if (bytesReceived == -1)
            {
                perror("recv failed");
                return FAIL;
            }
            recievedBuffLs[MAX_LENGTH] = 0;
            printf("%s",recievedBuffLs );
        }
        else if(strcmp(answer,"nok")==0){
            bytesReceived = recv(state.sock_fd,recievedBuffLs,MAX_LENGTH,0 );
            if (bytesReceived == -1)
            {
                perror("recv failed");
                return FAIL;
            }
            recievedBuffLs[MAX_LENGTH] = 0;
            cmd_line* rec = parse_cmd_lines(recievedBuffLs);
            fprintf(stderr, "Server Error: %s\n", rec->arguments[1]);
            bye();
        }
    }

    else{
        printf("can't ls cause you are NOT CONNECTED!\n");
        return BYE_FAIL_NOT_CONNECTED;
    }
    return RETURN_NORMAL;
}

int exec(cmd_line* parsedCmdLine){
    int ans_from_func = RETURN_NORMAL;
    if(strcmp(parsedCmdLine->arguments[0], "conn")==0)
        ans_from_func = conn(parsedCmdLine);
    else if(strcmp(parsedCmdLine->arguments[0], "bye")==0)
        ans_from_func = bye();
    else if(strcmp(parsedCmdLine->arguments[0], "ls")==0)
        ans_from_func = ls();
    return ans_from_func;
}

int main (int argc , char* argv[]){
    cmd_line* parsedCmdLine;
    char inputLine[MAX_LENGTH];
    memset(inputLine, 0, sizeof(inputLine));

    if(argc > 1 && strcmp("-d",argv[1]) == 0)
        d_flag = 1;
    
    while(TRUE){
        printf("server: %s>",state.server_addr);
        fgets(inputLine,MAX_LENGTH,stdin);
        if (strcmp("quit\n",inputLine) == 0){
            return RETURN_NORMAL;
        }
        if (strcmp("\n",inputLine) == 0)
            continue;
        parsedCmdLine = parse_cmd_lines(inputLine);
        if(exec(parsedCmdLine) != RETURN_NORMAL){
            perror("Error occured");
            return FAIL;
        }
    }
    free_cmd_lines(parsedCmdLine);

    return RETURN_NORMAL;
}