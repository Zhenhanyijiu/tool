#include "channel.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
// #include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
struct Channel
{
    int socket_fd;
    int conn;
};
void *initChannel(PartType pltype, char *address, int port)
{
    if (pltype == SENDER)
    {
        int socket_fd = 0; //socket句柄
        unsigned int iRemoteAddr = 0;
        struct sockaddr_in stRemoteAddr = {0}; //对端，即目标地址信息

        socket_fd = socket(AF_INET, SOCK_STREAM, 0); //建立socket
        if (0 > socket_fd)
        {
            printf("创建socket失败！\n");
            return nullptr;
        }

        stRemoteAddr.sin_family = AF_INET;
        stRemoteAddr.sin_port = htons(port);
        // inet_pton(AF_INET, ADDR, &iRemoteAddr);
        // stRemoteAddr.sin_addr.s_addr = iRemoteAddr;
        stRemoteAddr.sin_addr.s_addr = inet_addr(address);

        //连接方法： 传入句柄，目标地址，和大小
        if (0 > connect(socket_fd, (struct sockaddr *)&stRemoteAddr, sizeof(stRemoteAddr)))
        {
            printf("连接失败 connect error！\n");
            close(socket_fd);
            //printf("connect failed:%d",errno);//失败时也可打印errno
            return nullptr;
        }
        // printf("连接成功！\n");
        // recv(iSocketFD, buf, sizeof(buf), 0);
        //将接收数据打入buf，参数分别是句柄，储存处，最大长度，其他信息（设为0即可）。 
        // printf("Received:%s\n", buf);
        Channel *ch = (Channel *)malloc(sizeof(Channel));
        if (ch == NULL)
        {
            close(socket_fd);
            return nullptr;
        }
        ch->socket_fd = -1;
        ch->conn = -1;
        ch->socket_fd = socket_fd;
        return ch;
    }
    if (pltype == RECEIVER)
    {
        //调用socket函数返回的文件描述符
        int socket_fd;
        //声明两个套接字sockaddr_in结构体变量，分别表示客户端和服务器
        struct sockaddr_in server_addr = {0};
        struct sockaddr_in clientAddr = {0};
        int addr_len = sizeof(clientAddr);
        int conn;
        //socket函数，失败返回-1
        //int socket(int domain, int type, int protocol);
        //第一个参数表示使用的地址类型，一般都是ipv4，AF_INET
        //第二个参数表示套接字类型：tcp：面向连接的稳定数据传输SOCK_STREAM
        //第三个参数设置为0
        if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("socket error\n");
            return nullptr;
        }
        //初始化服务器端的套接字，并用htons和htonl将端口和地址转成网络字节序
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        //ip可是是本服务器的ip，也可以用宏INADDR_ANY代替，代表0.0.0.0，表明所有地址
        // server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        unsigned int localAddr = 0;
        inet_pton(AF_INET, address, &localAddr);
        // server_addr.sin_addr.s_addr =localAddr;
        // inet_addr("127.0.0.1");
        server_addr.sin_addr.s_addr = inet_addr(address);
        //对于bind，accept之类的函数，里面套接字参数都是需要强制转换成(struct sockaddr *)
        //bind三个参数：服务器端的套接字的文件描述符，
        if (bind(socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            printf("connect bind error\n");
            close(socket_fd);
            return nullptr;
        }
        //设置服务器上的socket为监听状态
        if (listen(socket_fd, 5) < 0)
        {
            printf("listen error\n");
            close(socket_fd);
            return nullptr;
        }
        conn = accept(socket_fd, (struct sockaddr *)&clientAddr, (socklen_t *)&addr_len);
        if (conn < 0)
        {
            printf("accept error\n");
            return nullptr;
        }
        Channel *ch = (Channel *)malloc(sizeof(Channel));
        if (ch == NULL)
        {
            close(socket_fd);
            close(conn);
            return nullptr;
        }
        ch->socket_fd = -1;
        ch->conn = -1;
        printf("accept ok...\n");
        ch->socket_fd = socket_fd;
        ch->conn = conn;
        return ch;
    }

    return nullptr;
}
int freeChannel(void *ch)
{
    if (ch)
    {
        Channel *c = (Channel *)ch;
        if (c->socket_fd >= 0)
        {
            close(c->socket_fd);
        }
        if (c->conn >= 0)
        {
            close(c->conn);
        }
        free(ch);
        ch = NULL;
    }
    return 0;
}