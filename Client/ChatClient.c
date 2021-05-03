#include <stdio.h>
#include <stdlib.h>
#include "UDPClient.h"

void recvMessage(char* message){
    printf("Server : %s", message);
}

void sendMessage(char* message, int bufSize){
    fgets(message, bufSize, stdin);
    printf("Message sent : %s", message);
}

// Driver code
int main(){
    struct ClientSide client;
    
    initializeClientConnection(&client, "192.168.0.2", 8080, 15);
    setClientFunctions(&client, 0, recvMessage, sendMessage);
    validateClientConnection(&client);
    
    return 0;
}
