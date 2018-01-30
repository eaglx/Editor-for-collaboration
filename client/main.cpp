#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
void serialize_msg(MESSAGE_INFO *, char *);
void deserialize_msg(char *, MESSAGE_INFO *);
#define PACKETSIZE sizeof(MESSAGE_INFO)
struct MESSAGE_INFO
{
    int flag;
    int posX;
    int posY;
    char chr;
};
#define FLAG_INSERT_BEFORE 111
#define FLAG_REPLACE 222
#define SEND_ALL_DATA 0
#define SEND_ERROR -1
#define RECIVE_ZERO 0
#define RECIVE_ERROR -1


int main()
{
    int socketDesc;
    struct sockaddr_in serverAddr;
    int nFoo = 1;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6666);
    MESSAGE_INFO msg;
    char bufferMSG[PACKETSIZE];

    if(connect(socketDesc, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
    {
        cout <<"#ERROR-client: Cannot connect to server!!!" << endl;
        return -1;
    }

    // 1. Download data. 2. Send insert (x2). 3. Print data.

    close(socketDesc);
    return 0;
}


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
