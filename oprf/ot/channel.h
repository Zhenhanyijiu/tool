#pragma once
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
