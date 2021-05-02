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
#include <json-c/json.h>
#include "UDPClient.h"

// Function to parse response header
int parseResponse(char* buffer){
    struct json_object* response;
    struct json_object* jsonSocket;
    struct json_object* status;
    int sNum = 0;

    response = json_tokener_parse(buffer);

    json_object_object_get_ex(response, "JSONSocket", &jsonSocket);
    json_object_object_get_ex(jsonSocket, "status", &status);
    sNum = json_object_get_int(status);

    return sNum;
}

// Function to create request header
void createRequest(char* buffer, int verClient){
    struct json_object* request;
    struct json_object* jsonSocket;
    struct json_object* version;
    version = json_object_new_int(1);

    request = json_object_new_object();
    jsonSocket = json_object_new_object();
    json_object_object_add(jsonSocket, "version", version);
    json_object_object_add(request, "JSONSocket", jsonSocket);
    const char* temp = json_object_to_json_string_ext(request, JSON_C_TO_STRING_PRETTY);
    for (int i = 0; i < strlen(temp); i++) {
        buffer[i] = temp[i];
    }
}

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

void setClientFunctions(struct ClientSide* this, int input, void (*recvFunct)(char*), void (*sendFunct)(char*, int)){
    this->input = input;
    this->recvFunct = recvFunct;
    this->sendFunct = sendFunct;
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
    FD_SET(this->input, &this->readfds);
    int activity = select( this->sockfd + 1 , &this->readfds , NULL , NULL , &this->timeout);
    if(activity <= 0){
        closeClientConnection(this);
        return 1;
    }
    return 0;
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
        if(FD_ISSET(this->input, &this->readfds)){
            int bufSize = 1024;
            char sentMsg[bufSize];
            
            this->sendFunct(sentMsg, sizeof(sentMsg));
            sendToServer(this, sentMsg);
        }
        // Receiving message
        if(FD_ISSET(this->sockfd, &this->readfds)){
            char* receivedMsg = receiveFromServer(this);
            this->recvFunct(receivedMsg);
        }
    }
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
           
    //if(setClientFds(this)) return;
    this->timeout.tv_sec = this->waitTime;
    this->timeout.tv_usec = 0;
    FD_ZERO(&this->readfds);
    FD_SET(this->sockfd, &this->readfds);
    int activity = select( this->sockfd + 1 , &this->readfds , NULL , NULL , &this->timeout);
    if(activity <= 0){
        closeClientConnection(this);
        return;
    }

    // Receiving response header
    if(FD_ISSET(this->sockfd, &this->readfds)){
        nBytes = recvfrom(this->sockfd, (char *)buffer, bufSize,
                          MSG_WAITALL, (struct sockaddr *) &this->servaddr,
                          &this->len);
        buffer[nBytes] = '\0';
        int support = parseResponse(buffer);
        if(support >= 200 && support < 300){
            this->valid = 1;
            useClientConnection(this);
        }
    }
}
