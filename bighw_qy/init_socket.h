#ifndef INIT_SOCKET_H
#define INIT_SOCKET_H
#include <netinet/in.h>

#define BACKLOG    100        /* 监听队列长 */
#define PORT    8080    /* web服务器监听端口 */

// 初始化web服务器，创建监听套接字
int init_socket(int *listen_fd, struct sockaddr_in *server_addr);


#endif
