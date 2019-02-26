//
//  init_socket.h
//  hello
//
//  Created by Jack on 2017/3/4.
//  Copyright © 2017年 Jack. All rights reserved.
//
/*
 * file:init_socket.h
 */
#ifndef INIT_SOCKET_H
#define INIT_SOCKET_H
#include <netinet/in.h>

#define BACKLOG    100        /* length of listening queue on socket */
#define PORT    8080    /* web server listening port */


/* initialize the socket on server, include below
 socket();
 bind();
 listen();
 */


/* listen_fd : the web server listen file decriptor
 server_addr: the web server ipv4 address
 RETURNS: success on 0, error on -1
 */
int init_socket(int *listen_fd, struct sockaddr_in *server_addr);


#endif
