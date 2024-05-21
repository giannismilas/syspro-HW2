#include <stdio.h>
#include <stdlib.h>
#include "functions.h"



int main(int argc, char** argv){
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




    return 0;
}