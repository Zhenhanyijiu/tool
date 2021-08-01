#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
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
typedef enum
{
    SENDER,
    RECEIVER
} PartType;
typedef struct Channel Channel;
void *initChannel(PartType pltype, const char *address, int port);
int send_data(void *channel, const char *buff, int buf_size);
int recv_data(void *channel, char *buff_output, int buf_size);
int freeChannel(void *ch);
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
typedef struct TimeComputeType
{
    struct timeval start;
    struct timeval end;
} TimeCompute;
void *newTimeCompute()
{
    return malloc(sizeof(TimeCompute));
}
void startTime(void *tc)
{
    assert(tc != nullptr);
    TimeCompute *t = (TimeCompute *)tc;
    gettimeofday(&(t->start), NULL);
}
//毫秒
int getEndTime(void *tc)
{
    assert(tc != nullptr);
    TimeCompute *t = (TimeCompute *)tc;
    gettimeofday(&(t->end), NULL);
    int time_use = (t->end.tv_sec - t->start.tv_sec) * 1000000 +
                   (t->end.tv_usec - t->start.tv_usec);
    return time_use / 1000;
}
void freeTimeCompute(void *tc)
{
    if (tc)
    {
        free(tc);
    }
}
struct Channel
{
    int socket_fd;
    int conn;
};
void *initChannel(PartType pltype, const char *address, int port)
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
        Timeout *t = new_timiout(3);
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
        ch->socket_fd = -1;
        ch->conn = -1;
        ch->conn = socket_fd;
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
        if (c->conn >= 0)
        {
            close(c->conn);
        }
        if (c->socket_fd >= 0)
        {
            close(c->socket_fd);
        }
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
    printf("===(send)conn:%d,sock:%d\n", chan->conn, chan->socket_fd);
    // ssize_t n = send(chan->conn, head, 4, 0);
    ssize_t n = send(chan->conn, (char *)&headlen, 4, 0);
    if (n < 0)
    {
        return -121;
    }
    n = send(chan->conn, buff, buf_size, 0);
    if (n < 0)
    {
        return -1;
    }
    return buf_size;
}
int recv_data(void *channel, char *buff_output, int buf_size)
{
    if (channel == NULL || buf_size <= 0)
    {
        return -122;
    }
    Channel *chan = (Channel *)channel;
    uint32_t headlen = 0;
    printf("===(recv)conn:%d,sock:%d\n", chan->conn, chan->socket_fd);
    int n = recv(chan->conn, (char *)&headlen, 4, 0);
    if (n != 4)
    {
        printf("recv head err:%d\n", n);
        return -110;
    }
    printf("===headlen:%d\n", headlen);
    n = recv(chan->conn, buff_output, headlen, 0);
    if (n < 0)
    {
        return -111;
    }
    return n;
}
#if 0
int main()
{
    Timeout *t = new_timiout(2);
    while (1)
    {
        if (is_timeout(&t))
        {
            printf("timeout!!\n");
            return 1;
        }
        continue;
    }

    return 0;
}
#endif
#if 0
int main(int argc, char *argv[])
{
    int port = 7777;
    char addr[] = "127.0.0.1";
    void *sender = NULL;
    void *recv = NULL;
    int ptype = atoi(argv[1]);
    if (ptype == SENDER) //0
    {
        sender = initChannel(SENDER, addr, port);
        if (sender)
        {
            printf("sender init ok...\n");
        }
        else
        {
            return -1;
        }
        char buff[] = "hello";
        int fg = send_data(sender, buff, 5);
        printf("===fg:%d\n", fg);
        char recv_buff[100] = {0};
        fg = recv_data(sender, recv_buff, 100);
        printf("===fg:%d,buff:%s\n", fg, recv_buff);
        // sleep(1);
        freeChannel(sender);
    }
    int len = sizeof(bool);
    printf("===>>len(bool):%d\n", len);
    if (ptype == RECEIVER)
    {
        recv = initChannel(RECEIVER, addr, port);
        if (recv)
        {
            printf("recv init ok...\n");
        }
        else
        {
            return -2;
        }
        char buff[100] = {0};
        int n = recv_data(recv, buff, 100);
        printf("===n:%d\n", n);
        printf("===buff:%s\n", buff);
        memcpy(buff + 5, " yyn", 4);
        send_data(recv, buff, 9);
        sleep(5);

        freeChannel(recv);
    }
    return 0;
}
#endif

#include <fstream>
#include <iostream>
using namespace std;
int write()
{
    ofstream fout;
    fout.open("", ios::out);
    assert(fout.is_open());
    int n = 0;
    fout.write("sssss", n);
    return 0;
}

char *readFileAllByC(const char *fileName, long int *size)
{
    FILE *fp = fopen(fileName, "rb");
    assert(fp);
    /* 获取文件大小 */
    fseek(fp, 0, SEEK_END);
    long int lSize = ftell(fp);
    rewind(fp);
    /* 分配内存存储整个文件 */
    char *buffer = (char *)malloc(sizeof(char) * lSize);
    if (buffer == NULL)
    {
        return NULL;
    }
    /* 将文件拷贝到buffer中 */
    long int n = fread(buffer, 1, lSize, fp);
    //这里也要关闭文件
    printf("==n:%ld,lSize:%ld\n", n, lSize);
    assert(n == lSize);
    /* 现在整个文件已经在buffer中，可由标准输出打印内容 */
    // printf("%s", buffer);

    /* 结束演示，关闭文件并释放内存 */
    *size = n;
    fclose(fp);
    return buffer;
}
char *readFileAllByCPP(const char *fileName, long int *sizeOut)
{
    // filebuf *pbuf;
    ifstream filestr;
    // char *buffer;
    // 要读入整个文件，必须采用二进制打开
    filestr.open(fileName, ios::binary);
    // 获取filestr对应buffer对象的指针
    filebuf *pbuf = filestr.rdbuf();
    // 调用buffer对象方法获取文件大小
    long size = pbuf->pubseekoff(0, ios::end, ios::in);
    pbuf->pubseekpos(0, ios::in);
    // 分配内存空间
    /* 分配内存存储整个文件 */
    char *buffer = (char *)malloc(sizeof(char) * size);
    if (buffer == NULL)
    {
        return NULL;
    }
    // 获取文件内容
    long n = pbuf->sgetn(buffer, size);
    assert(n == size);
    *sizeOut = size;
    filestr.close();
    // 输出到标准输出
    // cout.write(buffer, size);
    return buffer;
}

void writeFileAllByCPP(const char *fileName, const char *buf, long int bufSize)
{
    // 文件输出流;
    ofstream out;
    // 要写入整个文件，必须采用二进制打开
    out.open(fileName, ios::binary);
    assert(out);
    out.write(buf, bufSize);

    out.close();
}

#if 1
int main()
{
    long int size = 0;
    void *timeC = newTimeCompute();
    startTime(timeC);
    char *buf = readFileAllByC("../../../500w_1.txt", &size);
    int useTime = getEndTime(timeC);
    printf("===use time:%dms\n", useTime);
    free(buf);
    //cpp
    startTime(timeC);
    buf = readFileAllByCPP("../../../500w_1.txt", &size);
    useTime = getEndTime(timeC);
    printf("===readFileAllByCPP use time:%dms,size:%ld\n", useTime, size);
    //memcpy
    char perid[18];
    char *left = buf;
    char *end = buf + size;
    startTime(timeC);
    int i = 0;
    for (; left < end; left += 19)
    {
        memcpy(perid, left, 18);
        i++;
    }
    useTime = getEndTime(timeC);
    printf("===memcpy use time:%dms,size:%ld,count:%d\n", useTime, size, i);
    printf("===>>test malloc...\n");
    float num = 9 * 1024 * 1024 * 1024;
    char *p = (char *)malloc(num);
    printf("===>>num:%ld,sizeof:%ld\n", (unsigned long int)num, sizeof(num));
    assert(p != NULL);
    return 0;
    printf("===>>写文件。。。\n");
    startTime(timeC);
    writeFileAllByCPP("../../../500w_1_w.txt", buf, size);
    useTime = getEndTime(timeC);
    printf("===writefile use time:%dms\n", useTime);
    free(buf);

    return 0;
}
#endif
#if 0
int main()
{
    ifstream fin;
    // fin.open("../../id.txt", ios::in);
    fin.open("../../500w_1.txt", ios::in);
    assert(fin.is_open());
    char space;
    char id[19] = {0};
    int i = 0;
    void *timeCompute = newTimeCompute();
    assert(timeCompute);
    startTime(timeCompute);
    filebuf *pbuf;
    while (1)
    {
        // i++;
        if (!fin.eof())
        {
            fin.read(id, 18);
            // id[18] = 0;
            int n = fin.gcount();

            if (n != 18)
            {
                printf("n<18,n:%d\n", n);
                break;
            }
            // printf("ok...(%s)i:%d\n", id, i);
            fin.seekg(1, ios::cur);
            i++;
            // printf("ok...i:%d\n", i);
        }
        else
        {
            cout << "1...eof\n";
            break;
        }
    }
    fin.close();
    int usetime = getEndTime(timeCompute);
    printf("usetime:%dms,i:%d\n", usetime, i);
    return 0;
}
#endif