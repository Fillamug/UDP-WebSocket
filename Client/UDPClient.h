#ifndef UDPCLIENT_H_
#define UDPCLIENT_H_

#include <arpa/inet.h>

struct ClientSide {
    int sockfd, len, valid, waitTime, version, input;
    struct sockaddr_in servaddr;
    struct timeval timeout;
    fd_set readfds;
    void (*recvFunct)(char*);
    void (*sendFunct)(char*, int);
};

int parseResponse(char* buffer);

void createRequest(char* buffer, int verClient);

void initializeClientConnection(struct ClientSide* this, char* serverIP, int port, int waitTime);

void setClientFunctions(struct ClientSide* this, int input, void (*recvFunct)(char*), void (*sendFunct)(char*, int));

void closeClientConnection(struct ClientSide* this);

int setClientFds(struct ClientSide* this);

void sendToServer(struct ClientSide* this, char* message);

char* receiveFromServer(struct ClientSide* this);

void useClientConnection(struct ClientSide* this);

void validateClientConnection(struct ClientSide* this);

#endif
