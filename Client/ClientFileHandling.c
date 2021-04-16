#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include "ClientFileHandling.h"

#define nofile "File Not Found!"

// Function to clear buffer
void clearBuf(char* b, int s)
{
    int i;
    for (i = 0; i < s; i++)
        b[i] = '\0';
}

// Function sending file
int sendFile(FILE* fp, char* buf, int s)
{
    int i, len;
    if (fp == NULL) {
        strcpy(buf, nofile);
        len = strlen(nofile);
        buf[len] = EOF;
        return 1;
    }

    char ch;
    for (i = 0; i < s; i++) {
        ch = fgetc(fp);
        buf[i] = ch;
        if (ch == EOF)
            return 1;
    }
    return 0;
}

// Function to receive file
int recvFile(char* file, char* buf, int s)
{
    int i;
    char ch;
    FILE* fp;
    fp = fopen(file, "w");
    for (i = 0; i < s; i++) {
        ch = buf[i];
        if (ch == EOF) {
            fclose(fp);
            return 1;
        }
        else {
            fputc(ch, fp);
        }
    }
    return 0;
}

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

// Function to delete file
void rmFile(char* file){
    char* temp = "rm ";
    char* command = malloc(strlen(temp)+strlen(file)+1);
    strcpy(command, temp);
    strcat(command, file);
    system(command);
}
