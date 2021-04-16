#ifndef SERVERFILEHANDLING_H_
#define SERVERFILEHANDLING_H_

void clearBuf(char* b, int s);

int sendFile(FILE* fp, char* buf, int s);

int recvFile(char* file, char* buf, int s);

int parseRequest(char* buffer);

void createResponse(char* buffer, int support, int valid);

void rmFile(char* file);

#endif
