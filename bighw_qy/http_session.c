#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>
#include "http_session.h"
#include "get_time.h"

// 完成http响应
int http_session(int *connect_fd, struct sockaddr_in *client_addr)
{
    char recv_buf[RECV_BUFFER_SIZE + 1];
    unsigned char send_buf[SEND_BUFFER_SIZE + 1];
    unsigned char file_buf[FILE_MAX_SIZE + 1];
    memset(recv_buf, '\0', sizeof(recv_buf));
    memset(send_buf, '\0', sizeof(send_buf));
    memset(file_buf, '\0', sizeof(file_buf));
    
    char uri_buf[URI_SIZE + 1];
    memset(uri_buf, '\0', sizeof(uri_buf));
    
    int read_bytes = 0;
    int send_bytes = 0;
    int file_size = 0;
    char *mime_type;
    int uri_status;
    memset(recv_buf, '\0', sizeof(recv_buf));

    if((read_bytes = recv(*connect_fd, recv_buf, RECV_BUFFER_SIZE, 0)) == 0)
    {
        return 0;
    }
    else if(read_bytes > 0)
    {
        if(is_http_protocol(recv_buf) == 0)
        {
            fprintf(stderr, "Not http protocol.\n");
            close(*connect_fd);
            return -1;
        }
        else
        {
            memset(uri_buf, '\0', sizeof(uri_buf));
            if(get_uri(recv_buf, uri_buf) == NULL)
            {
                uri_status = URI_TOO_LONG;
                
            }
            else
            {
                printf("\nURI:%s\n", uri_buf);
                uri_status = get_uri_status(uri_buf);
                switch(uri_status)
                {
                    case FILE_OK:
                        printf("file ok\n");
                        mime_type = get_mime_type(uri_buf);
                        printf("mime type: %s\n", mime_type);
                        file_size = get_file_disk(uri_buf, file_buf);
                        send_bytes = reply_normal_information(send_buf, file_buf, file_size, mime_type);
                        break;
                    case FILE_NOT_FOUND:
                        printf("in switch on case FILE_NOT_FOUND\n");
                        send_bytes = set_error_information(send_buf, FILE_NOT_FOUND);
                        break;
                    case FILE_FORBIDEN:
                        break;
                    case URI_TOO_LONG:
                        break;
                    default:
                        break;
                }
                send(*connect_fd, send_buf, send_bytes, 0);
            }
        }
    }
    
    return 0;
}



// 判断是否是http请求
int is_http_protocol(char *msg_from_client)
{
    return 1;
    
    int index = 0;
    while(msg_from_client[index] != '\0' && msg_from_client[index] != '\n')
    {
        index++;
        printf("%d%c",index - 1, msg_from_client[index - 1]);
    }
    if(strncmp(msg_from_client + index - 10, "HTTP/", 5) == 0)
    {
        return 1;
    }
    
    return 0;
}

// 获得请求uri
char *get_uri(char *req_header, char *uri_buf)
{
    int index = 0;
    while( (req_header[index] != '/') && (req_header[index] != '\0') )
    {
        index++;
    }
    int base = index;
    while( ((index - base) < URI_SIZE) && (req_header[index] != ' ') && (req_header[index] != '\0') )
    {
        index++;
    }
    if( (index - base) >= URI_SIZE)
    {
        fprintf(stderr, "error: too long of uri request.\n");
        return NULL;
    }
    if((req_header[index - 1] == '/') && (req_header[index] == ' '))
    {
        strcpy(uri_buf, "index.html");
        return uri_buf;
    }
    strncpy(uri_buf, req_header + base + 1, index - base - 1);
    return uri_buf;
    
}

// 获得uri当前状态
int get_uri_status(char *uri)
{
    if(access(uri, F_OK) == -1)
    {
        fprintf(stderr, "File: %s not found.\n", uri);
        return FILE_NOT_FOUND;
    }
    if(access(uri, R_OK) == -1)
    {
        fprintf(stderr, "File: %s can not read.\n", uri);
        return FILE_FORBIDEN;
    }
    return FILE_OK;
}

// 获得请求类型
char *get_mime_type(char *uri)
{
    int len = strlen(uri);
    int dot = len - 1;
    while( dot >= 0 && uri[dot] != '.')
    {
        dot--;
    }
    if(dot == 0)
    {
        return NULL;
    }
    if(dot < 0)
    {
        return "text/html";
    }
    dot++;
    int type_len = len - dot;
    char *type_off = uri + dot;
    switch(type_len)
    {
        case 4:
            if(!strcmp(type_off, "html") || !strcmp(type_off, "HTML"))
            {
                return "text/html";
            }
            if(!strcmp(type_off, "jpeg") || !strcmp(type_off, "JPEG"))
            {
                return "image/jpeg";
            }
            break;
        case 3:
            if(!strcmp(type_off, "htm") || !strcmp(type_off, "HTM"))
            {
                return "text/html";
            }
            if(!strcmp(type_off, "css") || !strcmp(type_off, "CSS"))
            {
                return "text/css";
            }
            if(!strcmp(type_off, "ico") || !strcmp(type_off, "ICO"))
            {
                return "image/ico";
            }
            if(!strcmp(type_off, "png") || !strcmp(type_off, "PNG"))
            {
                return "image/png";
            }
            if(!strcmp(type_off, "jpg") || !strcmp(type_off, "JPG"))
            {
                return "image/jpeg";
            }
            if(!strcmp(type_off, "gif") || !strcmp(type_off, "GIF"))
            {
                return "image/gif";
            }
            if(!strcmp(type_off, "txt") || !strcmp(type_off, "TXT"))
            {
                return "text/plain";
            }
            break;
        case 2:
            if(!strcmp(type_off, "js") || !strcmp(type_off, "JS"))
            {
                return "text/javascript";
            }
            break;
        default:
            return "NULL";
            break;
    }
    
    return NULL;
}

// 获得文件
int get_file_disk(char *uri, unsigned char *file_buf)
{
    int read_count = 0;
    int fd = open(uri, O_RDONLY);
    if(fd == -1)
    {
        perror("open() in get_file_disk http_session.c");
        return -1;
    }
    unsigned long st_size;
    struct stat st;
    if(fstat(fd, &st) == -1)
    {
        perror("stat() in get_file_disk http_session.c");
        return -1;
    }
    st_size = st.st_size;
    if(st_size > FILE_MAX_SIZE)
    {
        fprintf(stderr, "the file %s is too large.\n", uri);
        return -1;
    }
    if((read_count = read(fd, file_buf, FILE_MAX_SIZE)) == -1)
    {
        perror("read() in get_file_disk http_session.c");
        return -1;
    }
    printf("file %s size : %lu , read %d\n", uri, st_size, read_count);
    close(fd);
    return read_count;
}


int set_error_information(unsigned char *send_buf, int errorno)
{
    register int index = 0;
    register int len = 0;
    char *str = NULL;
    switch(errorno)
    {
            
        case FILE_NOT_FOUND:
            printf("In set_error_information FILE_NOT_FOUND case\n");
            str = "HTTP/1.1 404 File Not Found\r\n";
            len = strlen(str);
            memcpy(send_buf + index, str, len);
            index += len;
            
            len = strlen(SERVER);
            memcpy(send_buf + index, SERVER, len);
            index += len;
            
            memcpy(send_buf + index, "\r\nDate:", 7);
            index += 7;
            
            char time_buf[TIME_BUFFER_SIZE];
            memset(time_buf, '\0', sizeof(time_buf));
            get_time_str(time_buf);
            len = strlen(time_buf);
            memcpy(send_buf + index, time_buf, len);
            index += len;
            
            str = "\r\nContent-Type:text/html\r\nContent-Length:";
            len = strlen(str);
            memcpy(send_buf + index, str, len);
            index += len;
            
            str = "\r\n\r\n404 File not found Please check your url,and try it again! ";
            len = strlen(str);
            int htmllen = len;
            char num_len[5];
            memset(num_len, '\0', sizeof(num_len));
            sprintf(num_len, "%d", len);
            
            len = strlen(num_len);
            memcpy(send + index, num_len, len);
            index += len;
            
            memcpy(send_buf + index, str, htmllen);
            index += htmllen;
            break;
            
            
        default:
            break;
            
    }
    return index;
}


int reply_normal_information(unsigned char *send_buf, unsigned char *file_buf, int file_size, char *mime_type)
{
    char *str = "HTTP/1.1 200 OK\r\nServer:Mutu/Linux(0.1)\r\nDate:";
    register int index = strlen(str);
    memcpy(send_buf, str, index);
    
    char time_buf[TIME_BUFFER_SIZE];
    memset(time_buf, '\0', sizeof(time_buf));
    str = get_time_str(time_buf);
    int len = strlen(time_buf);
    memcpy(send_buf + index, time_buf, len);
    index += len;
    
    len = strlen(ALLOW);
    memcpy(send_buf + index, ALLOW, len);
    index += len;
    
    memcpy(send_buf + index, "\r\nContent-Type:", 15);
    index += 15;
    len = strlen(mime_type);
    memcpy(send_buf + index, mime_type, len);
    index += strlen(mime_type);
    
    memcpy(send_buf + index, "\r\nContent-Length:", 17);
    index += 17;
    char num_len[8];
    memset(num_len, '\0', sizeof(num_len));
    sprintf(num_len, "%d", file_size);
    len = strlen(num_len);
    memcpy(send_buf + index, num_len, len);
    index += len;
    
    memcpy(send_buf + index, "\r\n\r\n", 4);
    index += 4;
    
    
    memcpy(send_buf + index, file_buf, file_size);
    index += file_size;
    return index;
    
}
