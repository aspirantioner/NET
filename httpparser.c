#include "httpparser.h"

void httpHeadextract(char *http_packet, struct http_request_head *head)
{
    sscanf(http_packet, "%s %s %s\r\n", head->method, head->file, head->version);
    return;
}

void setResHead(struct http_response_head *res_head, char *version, int status_code)
{
    strcpy(res_head->version, version);
    switch (status_code)
    {
    case 200:
        strcpy(res_head->status_code, "200");
        strcpy(res_head->status_name, "OK");
        break;
    default:
        break;
    }
    return;
}

void setResConnection(struct http_response *res, char *connection_type)
{
    strcpy(res->connection_type, connection_type);
    return;
}
void setResContentType(struct http_response *res, char *content_type)
{
    strcpy(res->content_type, content_type);
}
void setResFile(struct http_response *res, char *file_name)
{
    strcpy(res->file_name, file_name);
}
void sendResPacket(struct http_response *res, int cli_fd)
{
    char file_format[20];
    sscanf(res->file_name, "%*[^.].%s", file_format);
    int ret = 0;
    /*get file bytes size*/
    char send_buf[FILE_WRITE_LEN];
    int file_fd = open(res->file_name, O_RDONLY);
    struct stat sb;
    stat(res->file_name, &sb);
    int sum = sb.st_size;
    char str[10];
    sprintf(str, "%d", sum);

    if (strcmp(file_format, "css") == 0)
    {
        setResContentType(res, "text/css");
        ret = sprintf(send_buf, "%s %s %s\r\nContent-Type: %s\r\nContent-Length: %s\r\nServer: Apache\r\n\r\n", res->res_head.version, res->res_head.status_code, res->res_head.status_name, res->content_type, str);
    }
    else
    {
        setResContentType(res, "text/html");
        ret = sprintf(send_buf, "%s %s %s\r\nContent-Type: %s\r\nConnection: %s\r\nKeep-Alive: timeout=5, max=1000\r\nContent-Length: %s\r\nServer: Apache\r\n\r\n", res->res_head.version, res->res_head.status_code, res->res_head.status_name, res->content_type, res->connection_type, str);
    }

    /*read send file bytes*/
    ret += read(file_fd, send_buf + ret, FILE_WRITE_LEN);
    write(cli_fd, send_buf, ret);

    close(file_fd);
}