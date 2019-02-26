/**********************************************************
*
* Author：　　　ShooterIT
* Datatime: 　　2016-11-6 19:22:42
* description：基于预线程化的并发echo服务器
*
**********************************************************/

#include "utils.h"
#include "rio.h"
#include "net.h"
#include "sembuf.h"

#define NTHREAD 8
#define SBSIZE  16

//基于信号量的缓冲区
sembuf_t sb;

//回声
void echo(int connfd)
{
    size_t n;
    char buf[MAXLINE];
    rio_t rio;
    Rio_initbuf(&rio,connfd); //初始化缓存
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){ //读网络数据
        printf("server received %d bytes.\n", (int)n);   //标准输出
        Rio_write(connfd, buf, n);                       //写网络数据
    }
}

//回声线程
void *echo_thread(void *argv)
{
     //获得进程id并分离线程
    Pthread_detach(Pthread_self());
    while (1) {
        int connfd = sembuf_remove(&sb); //获得一个可用消费项
        //回声服务
        echo(connfd);
        //关闭连接
        Close(connfd);
    }

    return NULL;
}

int main(int argc, char *argv[])
{
    int i,listenfd,connfd;               //监听、连接套接字描述符
    socklen_t clientlen;                //套接字长度类型
    struct sockaddr_storage clientaddr; //适应于任何地址
    char client_hostname[MAXLINE];      //主机名
    char client_port[MAXLINE];          //端口
    pthread_t tid;                      //线程id

    //usage
    if(argc != 2){
        fprintf(stderr, "usage: %s <port>.\n", argv[0]);
        exit(0);
    }

    clientlen = sizeof(struct sockaddr_storage);

    //创建监听套接字描述符
    listenfd = Open_listenfd(argv[1]);

    //初始化缓冲区
    sembuf_init(&sb, SBSIZE);
    //创建线程
    for(i = 0; i < NTHREAD; i++){
        Pthread_create(&tid, NULL, echo_thread, NULL);
    }
    while (1) {
        connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        //获得客户端的信息
        Getnameinfo((SA*)&clientaddr, clientlen, client_hostname,MAXLINE,
            client_port, MAXLINE, 0);
        printf("Connected to (%s, %s).\n", client_hostname, client_port);
        //添加消费项
        sembuf_insert(&sb, connfd);
    }
    return 0;
}
