#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json-c/json.h>
#include "ServerFileHandling.h"

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
