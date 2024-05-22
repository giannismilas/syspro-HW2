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
    
    if(argc < 4) {
        fprintf(stderr, "Usage: %s hostname port command\n", argv[0]);
        exit(1);
    }

    char* server_name = argv[1];
    int port_num = atoi(argv[2]);
    char command[BUFFER_SIZE];
    int offset = 0;

    for (int i = 3; i < argc; i++) 
        offset += sprintf(command + offset, "%s ", argv[i]);

    printf("Command to send: %s\n", command);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("Socket error\n");

    server = gethostbyname(server_name);
    if (server == NULL) 
        error("No such host\n");

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr_list[0]);
    serv_addr.sin_port = htons(port_num);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    n = write(sockfd, command, strlen(command));
    if (n < 0)
        error("ERROR writing to socket");

    // Wait for the response from the server
    char response[BUFFER_SIZE];
    memset(response, 0, BUFFER_SIZE);
    n = read(sockfd, response, BUFFER_SIZE);
    if (n < 0)
        error("ERROR reading from socket");

    printf("Server response: %s\n", response);

    close(sockfd);
    return 0;
}
