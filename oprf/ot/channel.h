#pragma once
typedef enum
{
    SENDER,
    RECEIVER
} PartType;
typedef struct Channel Channel;
void *initChannel(PartType pltype, char *address, int port);
int freeChannel(void *ch);
