#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "functions.h"


int main(int argc, char** argv){
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    
    if(argc < 4) {                                                                  //check if args are enough and command is correct
        fprintf(stderr, "Usage: %s hostname port command\n", argv[0]);
        exit(1);
    }
    if(strcmp(argv[3],"setConcurrency") && strcmp(argv[3],"issueJob") && strcmp(argv[3],"poll") && strcmp(argv[3],"exit") && strcmp(argv[3],"stop")){
        printf("No <%s> command\n",argv[3]);
        return 1;
    }
    char* server_name = argv[1];
    int port_num = atoi(argv[2]);
    char command[BUFFER_SIZE];                                                      //put the user command into a string
    int offset = 0;
    offset += sprintf(command + offset, "%s", argv[3]);
    for (int i = 4; i < argc; i++) 
        offset += sprintf(command + offset, " %s", argv[i]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);                                       //initialize socket
    if (sockfd < 0)
        error("Socket error\n");

    server = gethostbyname(server_name);                                            //get server
    if (server == NULL) 
        error("No such host\n");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr_list[0]);
    serv_addr.sin_port = htons(port_num);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)     //connect to server to send the user command
        error("ERROR connecting");

    n = write(sockfd, command, strlen(command));                                    //write the command to the server socket
    if (n < 0)
        error("ERROR writing to socket");


    char response[BUFFER_SIZE];                                                     //wait for response
    while(1){
        memset(response, 0, BUFFER_SIZE);
        n = read(sockfd, response, BUFFER_SIZE);
        if(n<=0)
            break;
        printf("%s\n", response);
    }

    if(!strcmp(argv[3],"issueJob") && strcmp(response,"SERVER TERMINATED BEFORE EXECUTION")){       //if a new job is sent do additional waiting for the output of the job
        while(1){
            memset(response, 0, BUFFER_SIZE);
            n = read(sockfd, response, BUFFER_SIZE);
            if(n<=0)
                break;
            printf("%s\n", response);
        }
    }

    close(sockfd);
    return 0;
}
