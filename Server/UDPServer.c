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

#define PORT 8080
#define MAXLINE 1024
#define VER_SERVER 1
#define WAIT 15

// Driver code
int main() {
    int sockfd, nBytes, activity, len;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
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
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.5");
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,
            sizeof(servaddr)) < 0 )
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    len = sizeof(cliaddr);

    support = 0;
    correct = 0;

    // Receiving request header
    clearBuf(buffer, MAXLINE);
    nBytes = recvfrom(sockfd, (char *)buffer, MAXLINE,
                      MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                      &len);
    buffer[nBytes] = '\0';
    support = parseRequest(buffer);
    if(support >=1 && support <= VER_SERVER) correct = 1;
    
    // Sending response header
    clearBuf(buffer, MAXLINE);
    createResponse(buffer, support, VER_SERVER);
    sendto(sockfd, (const char *)buffer, strlen(buffer),
                    MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                    len);

    // Sending & receiving messages
    if(correct){
        while(1) {
            // Setting timer
            FD_ZERO(&readfds);
            FD_SET(sockfd, &readfds);
            activity = select( sockfd + 1 , &readfds , NULL , NULL , &timeout);
            if(activity < 0){
                close(sockfd);
                return 0;
            }

            if(activity){
                // Receiving message
                clearBuf(buffer, MAXLINE);
                nBytes = recvfrom(sockfd, (char *)buffer, MAXLINE,
                                  MSG_WAITALL, ( struct sockaddr *) &cliaddr,
                                  &len);
                buffer[nBytes] = '\0';
                printf("Client : %s", buffer);
                // Sending message
                sendto(sockfd, (const char *)buffer, strlen(buffer),
                       MSG_CONFIRM, (const struct sockaddr *) &cliaddr,
                       len);
                printf("Response sent.\n");

                if(strcmp(buffer, "exit\n") == 0){
                    break;
                }
            }
            else break;
        }

        close(sockfd);
    }

    return 0;
}
