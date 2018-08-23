#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ctype.h>
#include "line_parser.h"
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include "common.h"
#define TRUE 1
#define MAX_LENGTH 2000
#define FAIL -1
#define RETURN_NORMAL 0
#define PORT_NUMBER "2018"
#define BACKLOG 10


char host_name[MAX_LENGTH];
char client_id_buf[MAX_LENGTH];
char recieveBuff[MAX_LENGTH];
client_state state = {host_name,IDLE,NULL,-1};
int client_id =1;
int d_flag = 0;

void debug(char* msg){
    if(d_flag)
        fprintf(stderr, "%s|Log: %s \n", state.server_addr, msg);
    return;
}

void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void reset_state(){
    state.sock_fd = -1;
    state.conn_state = IDLE;
    state.client_id = NULL;
    return;
}

int main (int argc , char* argv[]){
    char inputLine[MAX_LENGTH];
    memset(inputLine, 0, sizeof(inputLine));
    
    if(argc > 1 && strcmp("-d",argv[1]) == 0)
        d_flag = 1;
    
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    
    memset(host_name, 0, sizeof(host_name));
    memset(client_id_buf, 0, sizeof(client_id_buf));
    gethostname(host_name, 1024);
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    if ((rv = getaddrinfo(NULL, PORT_NUMBER, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
            perror("Server -> socket");
            continue;
        }
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
            perror("Server -> setsockopt");
            exit(1);
        }
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("Server -> bind");
            continue;
        }
        break;
    } // trying to bind the first ai object
    
    freeaddrinfo(servinfo);
    
    if (p == NULL)  {
        fprintf(stderr, "Server: failed to bind\n");
        exit(1);
    } // cause we ran over all the ai_next's

    if (listen(sockfd, BACKLOG) == -1) {
        perror("Listen error");
        exit(1);
    }

    printf("Server is waiting for connections..\n");
    
    while(TRUE){
        sin_size = sizeof their_addr;
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
            perror("accept error");
            continue;
        }
        inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
        printf("Server got connection from address %s\n", s);
        
        while(TRUE){
            memset(recieveBuff, 0, sizeof(recieveBuff));
            int bytes_received = recv(new_fd, recieveBuff, MAX_LENGTH, 0);
            recieveBuff[bytes_received] = 0;
            
            debug(recieveBuff);
            
            if(strcmp(recieveBuff, "hello") == 0){
                    if(state.conn_state != IDLE){
	                perror("Client is already connected");
                        char * msgToSend = "nok state(already connected)";
                        if (send(new_fd, msgToSend, strlen(msgToSend), 0) == -1){
                            perror("nok state(already_connected) send failed");
                        }
                    }
                    else{
                        char msgToSend[MAX_LENGTH];
                        memset(msgToSend,0,sizeof(msgToSend));
                        sprintf(msgToSend, "hello %d", client_id);
                        if (send(new_fd, msgToSend, strlen(msgToSend), 0) == -1){
                            perror("hello send failed");
                        }
                        sprintf(client_id_buf, "%d", client_id);
                        printf("Client %s connected\n", client_id_buf);

                        state.conn_state = CONNECTED;
                        state.client_id = client_id_buf;
                        state.sock_fd = new_fd;
                        
                    }
            }
            
            else if(strcmp(recieveBuff, "bye") == 0){
                if(state.conn_state != CONNECTED){
                    perror("Client is not connected");
                    char * msg = "nok state(already_disconnected)";
                    if (send(new_fd, msg, strlen(msg), 0) == -1){
                        perror("nok state(not connected) send failed");
                    }
                }
                else{
                    client_id++;
                    reset_state();
                    printf("Client %s disconnected\n", client_id_buf);
                    close(new_fd);
                    break;
                }

            }

            else if(strcmp(recieveBuff, "ls") == 0){
            	if(state.conn_state != CONNECTED){
            		perror("Client is not connected");
                    char * msg = "nok state(already_disconnected)";
                    if (send(new_fd, msg, strlen(msg), 0) == -1){
                        perror("nok state(not connected) send failed");
                        return FAIL;
                    }
            	}
            	else{
            		char* msgToSend = "ok";
                    if (send(new_fd, msgToSend, strlen(msgToSend), 0) == -1){
                        perror("ok send failed");
                        return FAIL;
                    }

                    char* listOfDirs = list_dir();
                    if(send(new_fd,listOfDirs,strlen(listOfDirs),0) == -1){
                    	perror("list of directories send failed");
                    	return FAIL;
                    }
                    char cwd[MAX_LENGTH];
                    memset(cwd,0,sizeof(cwd));
                    cwd[MAX_LENGTH] = 0;
                    getcwd(cwd,MAX_LENGTH);
                    printf("Listed files at %s\n",cwd);
            	}
            }

            else{
                fprintf(stderr, "%s|Error: Unknown message %s\n", state.client_id, recieveBuff);
            }
            
        }//WHILE
        
    }//WHILE
        
        return RETURN_NORMAL;
}