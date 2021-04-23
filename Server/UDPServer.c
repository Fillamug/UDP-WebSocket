// Server side implementation of JSONSocket
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
#include "ServerFileHandling.h"

struct ServerSide {
    int sockfd, len, valid, waitTime, version;
    struct sockaddr_in servaddr, cliaddr;
    struct timeval timeout;
    fd_set readfds;
};

void initializeServerConnection(struct ServerSide* this, char* serverIP, int waitTime){
    this->valid = 0;
    this->waitTime = waitTime;
    this->version = 1;
    
    // Creating socket file descriptor
    if ( (this->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    memset(&this->servaddr, 0, sizeof(this->servaddr));
    memset(&this->cliaddr, 0, sizeof(this->cliaddr));
    
    // Filling server information
    this->servaddr.sin_family = AF_INET; // IPv4
    this->servaddr.sin_addr.s_addr = inet_addr(serverIP);
    this->servaddr.sin_port = htons(8080);
    
    // Bind the socket with the server address
    if ( bind(this->sockfd, (const struct sockaddr *)&this->servaddr,
            sizeof(this->servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    this->len = sizeof(this->cliaddr);
}

void closeServerConnection(struct ServerSide* this){
    close(this->sockfd);
    this->valid = 0;
}

int setServerFds(struct ServerSide* this){
    this->timeout.tv_sec = this->waitTime;
    this->timeout.tv_usec = 0;
    FD_ZERO(&this->readfds);
    FD_SET(this->sockfd, &this->readfds);
    FD_SET(0, &this->readfds);
    int activity = select( this->sockfd + 1 , &this->readfds , NULL , NULL , &this->timeout);
    if(activity <= 0){
        closeServerConnection(this);
        return 1;
    }
    return 0;
}

void validateServerConnection(struct ServerSide* this){
    int bufSize = 1024;
    int nBytes;
    char buffer[bufSize];
    
    // Receiving request header
    nBytes = recvfrom(this->sockfd, (char *)buffer, bufSize,
                      MSG_WAITALL, ( struct sockaddr *) &this->cliaddr,
                      &this->len);
    buffer[nBytes] = '\0';
    int support = parseRequest(buffer);
    if(support >=1 && support <= this->version) this->valid = 1;
    else{
        closeServerConnection(this);
        return;
    }
    
    // Sending response header
    clearBuf(buffer, bufSize);
    createResponse(buffer, support, this->valid);
    sendto(this->sockfd, (const char *)buffer, strlen(buffer),
                    MSG_CONFIRM, (const struct sockaddr *) &this->cliaddr,
                    this->len);
}

void sendToClient(struct ServerSide* this, char* message){
    sendto(this->sockfd, (const char *)message, strlen(message),
                       MSG_CONFIRM, (const struct sockaddr *) &this->cliaddr,
                       this->len);
}

char* receiveFromClient(struct ServerSide* this){
    int bufSize = 1024;
    char buffer[bufSize];
    int nBytes = recvfrom(this->sockfd, (char *)buffer, bufSize,
                      MSG_WAITALL, (struct sockaddr *) &this->cliaddr,
                      &this->len);
    buffer[nBytes] = '\0';
    char* message = buffer;
    return message;
}

void useServerConnection(struct ServerSide* this){
    if(this->valid){
        while(1){
            // Setting file descriptors
            if(setServerFds(this)) break;
            
            // Sending message
            if(FD_ISSET(0, &this->readfds)){
                char sentMsg[1024];
                fgets(sentMsg, sizeof(sentMsg), stdin);

                sendToClient(this, sentMsg);
                printf("Message sent : %s\n", sentMsg);
                
                if(strcmp(sentMsg, "exit\n") == 0){
                    break;
                }
            }
            // Receiving message
            if(FD_ISSET(this->sockfd, &this->readfds)){
                char* receivedMsg = receiveFromClient(this);
                printf("Client : %s", receivedMsg);

                if(strcmp(receivedMsg, "exit\n") == 0){
                    break;
                }
            }
        }
    }
}

// Driver code
int main(){
    struct ServerSide server;
    
    initializeServerConnection(&server, "192.168.0.2", 15);
    validateServerConnection(&server);
    useServerConnection(&server);
    
    return 0;
}
