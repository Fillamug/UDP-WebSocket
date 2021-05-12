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
#include <json-c/json.h>
#include <pthread.h>
#include "UDPServer.h"

// Function to parse request header
int parseRequest(char* buffer){
    struct json_object* response;
    struct json_object* jsonSocket;
    struct json_object* version;
    int vNum = 0;

    response = json_tokener_parse(buffer);

    json_object_object_get_ex(response, "JSONSocket", &jsonSocket);
    json_object_object_get_ex(jsonSocket, "version", &version);
    vNum = json_object_get_int(version);

    return vNum;
}

// Function to create response header
void createResponse(char* buffer, int support, int valid){
    struct json_object* response;
    struct json_object* jsonSocket;
    struct json_object* status;
    struct json_object* message;
    struct json_object* version;
    if(valid){
        status = json_object_new_int(200);
        message = json_object_new_string("OK");
        version = json_object_new_int(support);
    }
    else{
        status = json_object_new_int(505);
        message = json_object_new_string("Version not supported");
        version = json_object_new_int(support);
    }
    response = json_object_new_object();
    jsonSocket = json_object_new_object();
    json_object_object_add(jsonSocket, "status", status);
    json_object_object_add(jsonSocket, "message", message);
    json_object_object_add(jsonSocket, "version", version);
    json_object_object_add(response, "JSONSocket", jsonSocket);
    const char* temp = json_object_to_json_string_ext(response, JSON_C_TO_STRING_PRETTY);
    strcpy(buffer, temp);
}

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

void setServerFunctions(struct ServerSide* this, int input, void (*recvFunct)(char*), void (*sendFunct)(char*, int)){
    this->input = input;
    this->recvFunct = recvFunct;
    this->sendFunct = sendFunct;
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
    FD_SET(this->input, &this->readfds);
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
        if(FD_ISSET(this->input, &this->readfds)){
            int bufSize = 1024;
            char sentMsg[bufSize];
            
            this->sendFunct(sentMsg, sizeof(sentMsg));
            sendToClient(this, sentMsg);
        }
        // Receiving message
        if(FD_ISSET(this->sockfd, &this->readfds)){
            char* receivedMsg = receiveFromClient(this);
            this->recvFunct(receivedMsg);
        }
    }
}

void* createSocket(void* server){
    struct ServerSide* this = server;
    
    setSocketInfo(this);
    
    // Connecting client to socket
    connect(this->sockfd, (const struct sockaddr *) &this->cliaddr, this->len);
    
    int bufSize = 1024;
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
        char buffer[bufSize];
    
        // Receiving request header
        int nBytes = recvfrom(this->sockfd, (char *)buffer, bufSize,
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
