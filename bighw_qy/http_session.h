
#ifndef HTTP_SESSION_H
#define HTTP_SESSION_H

#include <netinet/in.h>


#define RECV_BUFFER_SIZE    1024        /* 1KB接收缓冲区 */
#define    SEND_BUFFER_SIZE    1050000        /* 1.xMB发送缓冲区 */
#define    URI_SIZE            128            /* 浏览器uri请求长度 */

#define    FILE_OK                200
#define    FILE_FORBIDEN        403            /* 无权限 */
#define FILE_NOT_FOUND        404            /* 未找到文件 */
#define    UNALLOW_METHOD        405            /* 不允许的http请求方法 */
#define FILE_TOO_LARGE        413            /* 文件太大 */
#define    URI_TOO_LONG        414            /* uri太长 */
#define    UNSUPPORT_MIME_TYPE    415            /* 不支持的文件类型 */
#define    UNSUPPORT_HTTP_VERSION    505            /* 不支持的http协议版本 */
#define    FILE_MAX_SIZE        1048576        /* 从硬盘读取的最大文件大小 */


#define ALLOW                "Allow:GET"    /* 目前只支持GET请求 */
#define    SERVER                "Server:Mutu(0.1 Alpha)/Linux"


/* 处理http请求 */
int http_session(int *connect_fd, struct sockaddr_in *client_addr);

/* 是http请求return 1, 否则return 0 */
int is_http_protocol(char *msg_from_client);

/* 获得请求头的uri */
char *get_uri(char *req_header, char *uri_buf);


/* 获得uri状态,通过return 0,不存在return 1,无权限return 2,错误return -1 */
int get_uri_status(char *uri);


/* 获得文件的mime类型 */
char *get_mime_type(char *uri);

/* 从硬盘读取文件 */
int get_file_disk(char *uri, unsigned char *entity_buf);

/* 设置返回头状态字段 */
int set_rep_status();

/* 设置错误信息 */
int set_error_information(unsigned char *send_buf, int errorno);

/* 设置一般信息 */
int reply_normal_information(unsigned char *send_buf, unsigned char *file_buf, int file_size, char *mime_type);

#endif
