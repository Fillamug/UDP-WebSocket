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

#define PORT 8080
#define MAXLINE 1024
#define VER_CLIENT 1
#define WAIT 15

// Driver code
int main() {
    int sockfd, nBytes, activity, len;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr;
    int support = 0;
    int correct = 0;
    fd_set readfds;
    struct timeval timeout;
    FILE* fp;

    timeout.tv_sec = WAIT;
    timeout.tv_usec = 0;

    // Creating socket file descriptor
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.2");

    len = sizeof(servaddr);

    // Sending request header
    clearBuf(buffer, MAXLINE);
    createRequest(buffer, VER_CLIENT);
    sendto(sockfd, (const char *)buffer, strlen(buffer),
           MSG_CONFIRM, (const struct sockaddr *) &servaddr,
           len);
           
    // Setting timer
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    activity = select( sockfd + 1 , &readfds , NULL , NULL , &timeout);
    if(activity < 0){
        close(sockfd);
        return 0;
    }

    // Receiving response header
    if(activity){
        clearBuf(buffer, MAXLINE);
        nBytes = recvfrom(sockfd, (char *)buffer, MAXLINE,
                          MSG_WAITALL, (struct sockaddr *) &servaddr,
                          &len);
        buffer[nBytes] = '\0';
        support = parseResponse(buffer);
        if(support >= 200 && support < 300) correct = 1;
    }
    else{
        close(sockfd);
        return 0;
    }

    // Sending & receiving messages
    if(correct){
        while(1){
            // Setting timer
            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);
            FD_SET(0, &readfds);
            activity = select( sockfd + 1 , &readfds , NULL , NULL , &timeout);
            if(activity < 0){
                close(sockfd);
                return 0;
            }

            // Sending message
            if(FD_ISSET(0, &readfds)){
                clearBuf(buffer, MAXLINE);
                fgets(buffer, sizeof(buffer), stdin);

                sendto(sockfd, (const char *)buffer, strlen(buffer),
                       MSG_CONFIRM, (const struct sockaddr *) &servaddr,
                       len);
                printf("Message sent : %s\n", buffer);
                
                if(strcmp(buffer, "exit\n") == 0){
                    break;
                }
            }
            // Receiving message
            if(FD_ISSET(sockfd, &readfds)){
                clearBuf(buffer, MAXLINE);
                nBytes = recvfrom(sockfd, (char *)buffer, MAXLINE,
                                  MSG_WAITALL, (struct sockaddr *) &servaddr,
                                  &len);
                buffer[nBytes] = '\0';
                printf("Server : %s", buffer);

                if(strcmp(buffer, "exit\n") == 0){
                    break;
                }
            }
        }
    }

    close(sockfd);
    return 0;
}
