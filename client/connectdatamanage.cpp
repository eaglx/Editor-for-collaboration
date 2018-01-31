#include "connectdatamanage.h"

ConnectDataManage::ConnectDataManage()
{
    connect_to_server();
}

ConnectDataManage::~ConnectDataManage()
{
    disconnect_client();
}

bool ConnectDataManage::connect_to_server()
{
    socketDesc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketDesc < 0)
    {
        // Save to file
        // << "#ERROR-client: Failed create socket!!!";
        return false;
    }

    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6666);

    if(connect(socketDesc, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
    {
        // << "#ERROR-client: Cannot connect to server!!!";
        return false;
    }

    return true;
}

void ConnectDataManage::disconnect_client()
{
    close(socketDesc);
}

void ConnectDataManage::serialize_msg(MESSAGE_INFO *msgPacket, char *data)
{
    int *q = (int *)data;
    *q = msgPacket->flag; q++;
    *q = msgPacket->posX; q++;
    *q = msgPacket->posY; q++;

    char *p = (char *)q;
    *p = msgPacket->chr; p++;
}

void ConnectDataManage::deserialize_msg(char *data, MESSAGE_INFO *msgPacket)
{
    int *q = (int *)data;
    msgPacket->flag = *q; q++;
    msgPacket->posX = *q; q++;
    msgPacket->posY = *q; q++;

    char *p = (char *)q;
    msgPacket->chr = *p; p++;
}
