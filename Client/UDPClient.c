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

void initializeClientConnection(struct ClientSide* this, char* serverIP, int waitTime){
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
    this->servaddr.sin_port = htons(8080);

    this->len = sizeof(this->servaddr);
}

void validateClientConnection(struct ClientSide* this){
    int bufSize = 1024;
    int nBytes;
    int activity;
    char buffer[bufSize];

    // Sending request header
    createRequest(buffer, this->version);
    sendto(this->sockfd, (const char *)buffer, strlen(buffer),
           MSG_CONFIRM, (const struct sockaddr *) &this->servaddr,
           this->len);
           
    // Setting timer
    this->timeout.tv_sec = this->waitTime;
    this->timeout.tv_usec = 0;
    FD_ZERO(&this->readfds);
    FD_SET(this->sockfd, &this->readfds);
    activity = select( this->sockfd + 1 , &this->readfds , NULL , NULL , &this->timeout);
    if(activity <= 0){
        close(this->sockfd);
        return;
    }

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

void useClientConnection(struct ClientSide* this){
    if(this->valid){
        int bufSize = 1024;
        int nBytes;
        int activity;
        char buffer[bufSize];
        
        while(1){
            // Setting timer
            this->timeout.tv_sec = this->waitTime;
            this->timeout.tv_usec = 0;
            FD_ZERO(&this->readfds);
            FD_SET(this->sockfd, &this->readfds);
            FD_SET(0, &this->readfds);
            activity = select( this->sockfd + 1 , &this->readfds , NULL , NULL , &this->timeout);
            if(activity <= 0){
                break;
            }

            // Sending message
            if(FD_ISSET(0, &this->readfds)){
                clearBuf(buffer, bufSize);
                fgets(buffer, sizeof(buffer), stdin);

                sendto(this->sockfd, (const char *)buffer, strlen(buffer),
                       MSG_CONFIRM, (const struct sockaddr *) &this->servaddr,
                       this->len);
                printf("Message sent : %s\n", buffer);
                
                if(strcmp(buffer, "exit\n") == 0){
                    break;
                }
            }
            // Receiving message
            if(FD_ISSET(this->sockfd, &this->readfds)){
                clearBuf(buffer, bufSize);
                nBytes = recvfrom(this->sockfd, (char *)buffer, bufSize,
                                  MSG_WAITALL, (struct sockaddr *) &this->servaddr,
                                  &this->len);
                buffer[nBytes] = '\0';
                printf("Server : %s", buffer);

                if(strcmp(buffer, "exit\n") == 0){
                    break;
                }
            }
        }
    }
}

void closeClientConnection(struct ClientSide* this){
    close(this->sockfd);
    this->valid = 0;
}

// Driver code
int main(){
    struct ClientSide client;
    
    initializeClientConnection(&client, "192.168.0.2", 15);
    validateClientConnection(&client);
    useClientConnection(&client);
    closeClientConnection(&client);
    
    return 0;
}
