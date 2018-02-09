#include "connectdatamanage.h"

void listen_from_server(MainWindow *w)
{
    logFile << "#INFO: thread listen_from_server started\n";
    w->sendMessage(5);

    /*
    MESSAGE_INFO msg;
    char bufferMSG[PACKETSIZE];
    std::string strBuffer;
    int byteGet;
    int length;
    */

    /*
     * Example
    ptr = (char*) bufferMSG;
    length = sizeof(bufferMSG)/sizeof(bufferMSG[0]);
    while(true)
    {
        byteGet = recv(socketDesc, bufferMSG, length, 0);
        if(byteGet < 0)
        {
            logFile << "#ERROR: thread listen_from_server recv";
            //close(socketDesc);
            //return -2;
        }
        else if(byteGet == 0) break;
        ptr += byteGet;
        length -= byteGet;
        if(length == 0) break;
    }
    deserialize_msg(bufferMSG, &msg);
    */
    logFile << "#INFO: thread listen_from_server stopped\n";
}

bool send_to_server()
{
    /*
    MESSAGE_INFO msg;
    char bufferMSG[PACKETSIZE];
    std::string strBuffer;
    int byteGet;
    int length;
    */

    /*
     * Example:
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
            logFile << "#ERROR: thread send_to_server send\n";
            //close(socketDesc);
            //return -3;
        }
        ptr += byteGet;
        length -= byteGet;
    }
    */
}
