#pragma once
// struct sender {

// };
typedef enum
{
    SENDER,
    RECEIVER
} PartType;
typedef struct Channel Channel;
void *initChannel(PartType pltype, char *address);

int freeChannel(Channel);
