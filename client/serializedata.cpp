#include "connectdatamanage.h"

void serialize_msg(MESSAGE_INFO *msgPacket, char *data)
{
    int *q = (int *)data;
    *q = msgPacket->flag; q++;
    *q = msgPacket->posX; q++;
    *q = msgPacket->posY; q++;

    char *p = (char *)q;
    *p = msgPacket->chr; p++;
}

void deserialize_msg(char *data, MESSAGE_INFO *msgPacket)
{
    int *q = (int *)data;
    msgPacket->flag = *q; q++;
    msgPacket->posX = *q; q++;
    msgPacket->posY = *q; q++;

    char *p = (char *)q;
    msgPacket->chr = *p; p++;
}
