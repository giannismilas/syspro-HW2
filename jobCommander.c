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
    if(argc<4){
        return 1;
    }


    char* server_name = argv[1];
    int port_num = atoi(argv[2]);
    char command[COMMANDSIZE];
    int offset=0;
    for (int i = 3; i < argc; i++) 
        offset += sprintf(command + offset, "%s ", argv[i]);
    printf("%s\n%d\n%s\n",server_name,port_num,command);




    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){ 
        printf("Socket error\n");
        return 1;
    }


    

    server = gethostbyname(server_name);
    if (server == NULL) {
        printf("No such host\n");
        return 1;
    }



    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr_list[0]);
    serv_addr.sin_port = htons(port_num);


    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("ERROR connecting");
        return 1;
    }


    n = write(sockfd, command, strlen(command));
    if (n < 0){
        printf("ERROR writing to socket");
        return 1;
    }

    return 0;
}