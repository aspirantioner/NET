#ifndef _HTTPPARSER_H
#define _HTTPPARSER_H

#define HTTP_HEAD_CONTENT_LEN 100
#define HTTP_ATTR_CONTENT_LEN 1000
#define FILE_WRITE_LEN 10000
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef struct http_request_head
{
    char method[HTTP_HEAD_CONTENT_LEN];
    char file[HTTP_HEAD_CONTENT_LEN];
    char version[HTTP_HEAD_CONTENT_LEN];
} http_request_head;

typedef struct http_response_head
{
    char version[HTTP_HEAD_CONTENT_LEN];
    char status_code[HTTP_HEAD_CONTENT_LEN];
    char status_name[HTTP_HEAD_CONTENT_LEN];
} http_response_head;

typedef struct http_response
{
    struct http_response_head res_head;
    char content_type[HTTP_HEAD_CONTENT_LEN];
    char connection_type[HTTP_HEAD_CONTENT_LEN];
    char file_name[HTTP_HEAD_CONTENT_LEN];
} http_response;

void setResHead(struct http_response_head *res_head, char *version, int status_code);
void setResConnection(struct http_response *res, char *connection_type);
void setResContentType(struct http_response *res, char *content_type);
void setResFile(struct http_response *res, char *file_name);
void sendResPacket(struct http_response *res, int cli_fd);
void httpHeadextract(char *http_packet, struct http_request_head *head);

#endif