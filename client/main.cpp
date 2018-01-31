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
#define PACKETSIZE sizeof(MESSAGE_INFO)
struct MESSAGE_INFO
{
    int flag;
    int posX;
    int posY;
    char chr;
};
void serialize_msg(MESSAGE_INFO *, char *);
void deserialize_msg(char *, MESSAGE_INFO *);
#define FLAG_INSERT_BEFORE 111
#define FLAG_REPLACE 222
#define FLAG_APPEND 333
#define SEND_ALL_DATA 0
#define SEND_ERROR -1
#define RECIVE_ZERO 0
#define RECIVE_ERROR -1

using namespace std;

int main()
{
    int socketDesc;
    struct sockaddr_in serverAddr;
    MESSAGE_INFO msg;
    char bufferMSG[PACKETSIZE];
    char buffer[50];
    string str;
    int byteGet;
    int length;

    socketDesc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketDesc < 0)
    {
        cout << "#ERROR-client: Failed create socket!!!" << endl;
        return -1;
    }

    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6666);

    if(connect(socketDesc, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
    {
        cout <<"#ERROR-client: Cannot connect to server!!!" << endl;
        return -1;
    }

    // 1. Download data. 2. Send insert (x2). 3. Print data.

    cout << "DOWNLOAD DATA" << endl;
    str = "";
    for(int i = 0; i < 50; i++) { buffer[i] = '\0'; }
    while(true)
    {
        byteGet = recv(socketDesc, &buffer, sizeof(char) * 50, 0);
        cout << "recv bytes " << byteGet << endl;
        if(byteGet < 0)
        {
            cout <<"#ERROR-client: recv" << endl;
            close(socketDesc);
            return -2;
        }
        else if(byteGet == 0) break;

        str = str + string(buffer);
        if(byteGet < int(sizeof(char) * 50)) break;
    }

    cout << "RECIVE DATA PRINT" << endl;
    cout << str << endl;
    cout << "****************************" << endl;

    cout << "START APPEND CHAR" << endl;
    msg.flag = FLAG_APPEND;
    msg.posX = 0;
    msg.posY = 0;
    msg.chr = 'A';
    serialize_msg(&msg, bufferMSG);
    length = sizeof(bufferMSG)/sizeof(bufferMSG[0]);
    char *ptr = (char*) bufferMSG;
    while(length > 0)
    {
        byteGet = send(socketDesc, ptr, length, 0);
        if(byteGet < 0)
        {
            cout <<"#ERROR-client: send" << endl;
            close(socketDesc);
            return -3;
        }
        ptr += byteGet;
        length -= byteGet;
    }
    /*
    cout << "START RECIVE MSG" << endl;
    ptr = (char*) bufferMSG;
    length = sizeof(bufferMSG)/sizeof(bufferMSG[0]);
    while(true)
    {
        byteGet = recv(socketDesc, bufferMSG, length, 0);
        if(byteGet < 0)
        {
            cout <<"#ERROR-client: recv" << endl;
            close(socketDesc);
            return -2;
        }
        else if(byteGet == 0) break;
        ptr += byteGet;
        length -= byteGet;
        if(length == 0) break;
    }
    deserialize_msg(bufferMSG, &msg);
    cout << "MSG RECIVED: " << endl;
    cout << msg.flag << endl;
    cout << msg.posX << endl;
    cout << msg.posY << endl;
    cout << msg.chr << endl;
    */

    cout << "FINISH CLIENT" << endl;

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
