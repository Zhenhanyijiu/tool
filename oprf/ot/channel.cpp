#include "channel.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
// #include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include "assert.h"
//30M
#define RECV_BUFF_SIZE 1024 * 1024 * 30
// #define RECV_BUFF_SIZE 256
typedef struct Timeout
{
    int is_first;
    int inerval_sec;
    struct timeval start;
    struct timeval end;

} Timeout;
Timeout *new_timiout(int second)
{
    Timeout *t = (Timeout *)malloc(sizeof(Timeout));
    if (t)
    {
        t->is_first = 1;
        t->inerval_sec = second;
    }
    return t;
}
int is_timeout(Timeout **tout)
{
    if (tout && *tout)
    {
        if ((*tout)->is_first)
        {
            (*tout)->is_first = 0;
            gettimeofday(&(*tout)->start, NULL);
            return 0;
        }
        else
        {
            gettimeofday(&(*tout)->end, NULL);
            int time_use = ((*tout)->end.tv_sec - (*tout)->start.tv_sec) * 1000000 +
                           ((*tout)->end.tv_usec - (*tout)->start.tv_usec);
            if (time_use >= (*tout)->inerval_sec * 1e6) //set time-out 1s
            {
                free(*tout);
                *tout = NULL;
                return 1;
            }
            return 0;
        }
    }
    return 1;
}

struct Channel
{
    int socket_fd;
    int conn;
    char *recv_buff;
    int recv_buff_len;
};
void *initChannel(PartType pltype, const char *address, int port)
{
    if (pltype == CLIENT)
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
        Timeout *t = new_timiout(300);
        while (1)
        {
            int fg = connect(socket_fd, (struct sockaddr *)&stRemoteAddr, sizeof(stRemoteAddr));
            if (fg < 0)
            {
                // printf("连接失败 connect error,重试中！\n");
                // close(socket_fd);
                // return nullptr;
                if (is_timeout(&t))
                {
                    printf("连接失败 connect error！\n");
                    close(socket_fd);
                    return nullptr;
                }
                continue;
            }
            else
            {
                printf("连接成功！\n");
                break;
            }
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
        ch->recv_buff = (char *)malloc(RECV_BUFF_SIZE); //100M
        if (ch->recv_buff == NULL)
        {
            free(ch);
            close(socket_fd);
            return nullptr;
        }
        ch->recv_buff_len = RECV_BUFF_SIZE;
        ch->socket_fd = -1;
        ch->conn = -1;
        ch->conn = socket_fd;
        memset(ch->recv_buff, 0, ch->recv_buff_len);
        return ch;
    }
    if (pltype == SERVER)
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
            close(conn);
            close(socket_fd);
            return nullptr;
        }
        ch->recv_buff = (char *)malloc(RECV_BUFF_SIZE); //100M
        if (ch->recv_buff == NULL)
        {
            free(ch);
            close(conn);
            close(socket_fd);
            return nullptr;
        }
        ch->recv_buff_len = RECV_BUFF_SIZE;
        ch->socket_fd = -1;
        ch->conn = -1;
        printf("accept ok...\n");
        ch->socket_fd = socket_fd;
        ch->conn = conn;
        memset(ch->recv_buff, 0, ch->recv_buff_len);
        return ch;
    }

    return nullptr;
}
int releaseChannel(void *ch)
{
    if (ch)
    {
        Channel *c = (Channel *)ch;
        if (c->conn >= 0)
        {
            close(c->conn);
        }
        if (c->socket_fd >= 0)
        {
            close(c->socket_fd);
        }
        free(c->recv_buff);
        c->recv_buff = nullptr;
        free(ch);
        ch = NULL;
    }
    return 0;
}

int send_data(void *channel, const char *buff, int buf_size)
{
    if (channel == NULL || buf_size < 0)
    {
        return -120;
    }
    if (buf_size == 0)
    {
        return 0;
    }
    uint32_t headlen = (uint32_t)buf_size;
    Channel *chan = (Channel *)channel;
    // char head[4] = {0};
    // memcpy(head, (char *)&headlen, 4);
    ssize_t n = send(chan->conn, (char *)&headlen, 4, 0);
    assert(n == 4);
    if (n != 4)
    {
        printf("send header error,n(%ld)\n", n);
        return -121;
    }
    int offset = 0;
    int remain_len = buf_size;
    while (1)
    {
        n = send(chan->conn, buff + offset, remain_len, 0);
        if (n < 0)
        {
            return -111;
        }
        offset += n;
        if (offset < buf_size)
        {
            remain_len = buf_size - offset;
            continue;
        }
        else
        {
            break;
        }
    }
    // n = send(chan->conn, buff, buf_size, 0);
    assert(n == buf_size);
    return n;
}
int recv_data(void *channel, char **buff_output)
{
    if (channel == NULL)
    {
        printf("error...1\n");
        return -122;
    }
    Channel *chan = (Channel *)channel;
    uint32_t headlen = 0;
    int n = recv(chan->conn, (char *)&headlen, 4, 0);
    if (n != 4)
    {
        printf("error...2。n(%d)\n", n);
        return -110;
    }
    //空间不够
    if (headlen > chan->recv_buff_len)
    {
        char *tmp_buf = (char *)realloc(chan->recv_buff, headlen);
        if (tmp_buf == NULL)
        {
            printf("error...3\n");
            return -112;
        }
        chan->recv_buff = tmp_buf;
        chan->recv_buff_len = headlen;
        memset(chan->recv_buff, 0, headlen);
    }
    //这里要循环接收数据
    int offset = 0;
    int remain_len = headlen;
    // int count = 0;
    while (1)
    {
        n = recv(chan->conn, chan->recv_buff + offset, remain_len, 0);
        if (n < 0)
        {
            printf("error...4,n(%d),headlen(%d)\n", n, headlen);
            return -111;
        }
        // count++;
        offset += n;
        if (offset < headlen)
        {
            remain_len = headlen - offset;
            continue;
        }
        else
        {
            break;
        }
    }
    *buff_output = chan->recv_buff;
    return headlen;
}