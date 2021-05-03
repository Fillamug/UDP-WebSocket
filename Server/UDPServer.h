#ifndef UDPSERVER_H_
#define UDPSERVER_H_

#include <arpa/inet.h>

struct ServerSide {
    int sockfd, len, valid, waitTime, version, support, input, maxClients;
    struct sockaddr_in servaddr, cliaddr;
    struct timeval timeout;
    fd_set readfds;
    void (*recvFunct)(char*);
    void (*sendFunct)(char*, int);
};

int parseRequest(char* buffer);

void createResponse(char* buffer, int support, int valid);

void setSocketInfo(struct ServerSide* this);

void initializeServerConnection(struct ServerSide* this, char* serverIP, int port, int waitTime, int maxClients);

void setServerFunctions(struct ServerSide* this, int input, void (*recvFunct)(char*), void (*sendFunct)(char*, int));

void closeServerConnection(struct ServerSide* this);

int setServerFds(struct ServerSide* this);

void sendToClient(struct ServerSide* this, char* message);

char* receiveFromClient(struct ServerSide* this);

void useServerConnection(struct ServerSide* this);

void* createSocket(void* server);

void validateServerConnection(struct ServerSide* this);

#endif
