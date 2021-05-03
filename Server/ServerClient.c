#include <stdio.h>
#include <stdlib.h>
#include "UDPServer.h"

void recvMessage(char* message){
    printf("Client : %s", message);
}

void sendMessage(char* message, int bufSize){
    fgets(message, bufSize, stdin);
    printf("Message sent : %s", message);
}

// Driver code
int main(){
    struct ServerSide server;
    
    initializeServerConnection(&server, "192.168.0.2", 8080, 15, 10);
    setServerFunctions(&server, 0, recvMessage, sendMessage);
    validateServerConnection(&server);
    
    return 0;
}
