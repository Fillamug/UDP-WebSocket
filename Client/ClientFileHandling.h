#ifndef CLIENTFILEHANDLING_H_
#define CLIENTFILEHANDLING_H_

void clearBuf(char* b, int s);

int sendFile(FILE* fp, char* buf, int s);

int recvFile(char* file, char* buf, int s);

int parseResponse(char* buffer);

void createRequest(char* buffer, int verClient);

void rmFile(char* file);

#endif
