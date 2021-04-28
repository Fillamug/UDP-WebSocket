// Client side implementation of SJONSocket
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <memory.h>
#include "ClientFileHandling.h"

struct ClientSide {
    int sockfd, len, valid, waitTime, version;
    struct sockaddr_in servaddr;
    struct timeval timeout;
    fd_set readfds;
};

void initializeClientConnection(struct ClientSide* this, char* serverIP, int port, int waitTime){
    this->valid = 0;
    this->waitTime = waitTime;
    this->version = 1;
    
    // Creating socket file descriptor
    if ( (this->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&this->servaddr, 0, sizeof(this->servaddr));

    // Filling server information
    this->servaddr.sin_family = AF_INET;
    this->servaddr.sin_addr.s_addr = inet_addr(serverIP);
    this->servaddr.sin_port = htons(port);

    this->len = sizeof(this->servaddr);
}

void closeClientConnection(struct ClientSide* this){
    close(this->sockfd);
    this->valid = 0;
}

int setClientFds(struct ClientSide* this){
    this->timeout.tv_sec = this->waitTime;
    this->timeout.tv_usec = 0;
    FD_ZERO(&this->readfds);
    FD_SET(this->sockfd, &this->readfds);
    FD_SET(0, &this->readfds);
    int activity = select( this->sockfd + 1 , &this->readfds , NULL , NULL , &this->timeout);
    if(activity <= 0){
        closeClientConnection(this);
        return 1;
    }
    return 0;
}

void validateClientConnection(struct ClientSide* this){
    int bufSize = 1024;
    int nBytes;
    char buffer[bufSize];

    // Sending request header
    createRequest(buffer, this->version);
    sendto(this->sockfd, (const char *)buffer, strlen(buffer),
           MSG_CONFIRM, (const struct sockaddr *) &this->servaddr,
           this->len);
           
    if(setClientFds(this)) return;

    // Receiving response header
    if(FD_ISSET(this->sockfd, &this->readfds)){
        clearBuf(buffer, bufSize);
        nBytes = recvfrom(this->sockfd, (char *)buffer, bufSize,
                          MSG_WAITALL, (struct sockaddr *) &this->servaddr,
                          &this->len);
        buffer[nBytes] = '\0';
        int support = parseResponse(buffer);
        if(support >= 200 && support < 300) this->valid = 1;
    }
}

void sendToServer(struct ClientSide* this, char* message){
    sendto(this->sockfd, (const char *)message, strlen(message),
                       MSG_CONFIRM, (const struct sockaddr *) &this->servaddr,
                       this->len);
}

char* receiveFromServer(struct ClientSide* this){
    int bufSize = 1024;
    char buffer[bufSize];
    int nBytes = recvfrom(this->sockfd, (char *)buffer, bufSize,
                      MSG_WAITALL, (struct sockaddr *) &this->servaddr,
                      &this->len);
    buffer[nBytes] = '\0';
    char* message = buffer;
    return message;
}

void useClientConnection(struct ClientSide* this){
    while(this->valid){
        // Setting file descriptors
        if(setClientFds(this)) break;
        
        // Sending message
        if(FD_ISSET(0, &this->readfds)){
            char sentMsg[1024];
            fgets(sentMsg, sizeof(sentMsg), stdin);
            
            sendToServer(this, sentMsg);
            printf("Message sent : %s\n", sentMsg);
            
            if(strcmp(sentMsg, "exit\n") == 0){
                break;
            }
        }
        // Receiving message
        if(FD_ISSET(this->sockfd, &this->readfds)){
            char* receivedMsg = receiveFromServer(this);
            printf("Server : %s", receivedMsg);
            
            if(strcmp(receivedMsg, "exit\n") == 0){
                break;
            }
        }
    }
}

// Driver code
int main(){
    struct ClientSide client;
    
    initializeClientConnection(&client, "192.168.0.2", 8080, 15);
    validateClientConnection(&client);
    useClientConnection(&client);
    
    return 0;
}
