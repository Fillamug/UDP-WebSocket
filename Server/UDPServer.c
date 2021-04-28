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
#include <pthread.h>
#include "ServerFileHandling.h"

struct ServerSide {
    int sockfd, len, valid, waitTime, version, support, maxClients;
    struct sockaddr_in servaddr, cliaddr;
    struct timeval timeout;
    fd_set readfds;
};

void setSocketInfo(struct ServerSide* this){
    // Creating socket file descriptor
    if ((this->sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    
    // Making port reusable
    int reuse = 1;
    if(setsockopt(this->sockfd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0){
        perror("socket option failed");
        exit(EXIT_FAILURE);
    }
    
    // Bind the socket with the server address
    if (bind(this->sockfd, (const struct sockaddr *)&this->servaddr, sizeof(this->servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

void initializeServerConnection(struct ServerSide* this, char* serverIP, int port, int waitTime, int maxClients){
    this->valid = 0;
    this->waitTime = waitTime;
    this->version = 1;
    this->support = 0;
    this->maxClients = maxClients;
    
    memset(&this->servaddr, 0, sizeof(this->servaddr));
    memset(&this->cliaddr, 0, sizeof(this->cliaddr));
    
    // Filling server information
    this->servaddr.sin_family = AF_INET; // IPv4
    this->servaddr.sin_addr.s_addr = inet_addr(serverIP);
    this->servaddr.sin_port = htons(port);

    this->len = sizeof(this->cliaddr);
    
    setSocketInfo(this);
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

void sendToClient(struct ServerSide* this, char* message){
    send(this->sockfd, (const char *)message, strlen(message),
                     MSG_CONFIRM);
}

char* receiveFromClient(struct ServerSide* this){
    int bufSize = 1024;
    char buffer[bufSize];
    int nBytes = recv(this->sockfd, (char *)buffer, bufSize,
                      MSG_WAITALL);
    buffer[nBytes] = '\0';
    char* message = buffer;
    return message;
}

void useServerConnection(struct ServerSide* this){
    while(this->valid){
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

void* createSocket(void* server){
    struct ServerSide* this = server;
    
    setSocketInfo(this);
    
    // Connecting client to socket
    connect(this->sockfd, (const struct sockaddr *) &this->cliaddr, this->len);
    
    int bufSize = 1024;
    int nBytes;
    char buffer[bufSize];
    
    // Sending response header
    createResponse(buffer, this->support, this->valid);
    send(this->sockfd, (const char *)buffer, strlen(buffer),
                     MSG_CONFIRM);
    useServerConnection(this);
}

void validateServerConnection(struct ServerSide* this){
    pthread_t threads[this->maxClients];
    for(int i=0; i<this->maxClients; i++){
        int bufSize = 1024;
        int nBytes;
        char buffer[bufSize];
    
        // Receiving request header
        nBytes = recvfrom(this->sockfd, (char *)buffer, bufSize,
                          MSG_WAITALL, ( struct sockaddr *) &this->cliaddr,
                          &this->len);
        buffer[nBytes] = '\0';
        this->support = parseRequest(buffer);
        if(this->support >=1 && this->support <= this->version){
            this->valid = 1;
            struct ServerSide* server = malloc(sizeof(struct ServerSide));
            memcpy(server, this, sizeof(struct ServerSide));
            pthread_create(&threads[i], NULL, createSocket, server);
            this->valid = 0;
        }
    }
    for(int i=0; i<this->maxClients; i++){
        pthread_join(threads[i], NULL);
    }
}

// Driver code
int main(){
    struct ServerSide server;
    
    initializeServerConnection(&server, "192.168.0.2", 8080, 15, 10);
    validateServerConnection(&server);
    
    return 0;
}
