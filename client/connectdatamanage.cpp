#include "connectdatamanage.h"

void listen_from_server(MainWindow *w)
{
    logFile << "#INFO: thread listen_from_server started\n";
    MESSAGE_INFO msg;
    char bufferMSG[PACKETSIZE];
    int byteGet;
    int length;
    bool errorRecv = false;
    pollfd myPoll[1];
    int readypoll;

    w->sendMessage(FLAG_UPDATE_FROM_SERV);
    myPoll[1].fd = socketDesc;
    myPoll[1].events = POLLIN;
#define POLL_FD_NUMBERS 2
#define POLL_TIMEOUT 1000
    while(!isEndProgram)
    {
        readypoll = poll(myPoll, POLL_FD_NUMBERS, POLL_TIMEOUT);
        std::unique_lock<std::mutex> lk(myMutex);
        myConditionVariable.wait(lk, []{return readyM_CV;});
        if(sendDATA == true) { sendDATA = false; continue; }
        if(readypoll <= 0) { continue; }
        else
        {
            length = sizeof(bufferMSG)/sizeof(bufferMSG[0]);
            while(true)
            {
                byteGet = recv(myPoll[1].fd, bufferMSG, length, 0);
                if(byteGet < 0)
                {
                    logFile << "#ERROR: thread listen_from_server recv";
                    //close(socketDesc);
                    //return -2;
                    break;
                    errorRecv = true;
                }
                else if(byteGet == 0) { break; errorRecv = true; }
                length -= byteGet;
                if(length == 0) break;
            }
            if(errorRecv) { errorRecv = false; continue; }
            deserialize_msg(bufferMSG, &msg);
            if(msg.flag == FLAG_INSERT_BEFORE)
            {
                dataFromServer.insert(msg.posX, std::string(1, msg.chr));
            }
            else if(msg.flag == FLAG_REPLACE)
            {
                //while(unsigned(msg.posX + 1) > dataFromServer.size())
                    //dataFromServer.resize(dataFromServer.size() + 1);
                dataFromServer[msg.posX] =  msg.chr; //.replace(msg.posX, msg.posX+1, std::string(1, msg.chr));
            }
            else if(msg.flag == FLAG_APPEND)
            {
                dataFromServer.append(std::string(1, msg.chr));
            }
            else if(msg.flag == FLAG_RM)
            {
                if(dataFromServer.length() > 0)
                    dataFromServer = dataFromServer.substr(0, (dataFromServer.size() - msg.posX));
                //if(dataFromServer.length() == 0)
                //    dataFromServer = "#Hey#";
            }
            else if(msg.flag == FLAG_DEL_ALL)
            {
                dataFromServer.clear();
                dataFromServer = "";
            }
            else if(msg.flag == FLAG_SEND_STRING)
            {
            #define BUFF_SIZE 50
                char buffer[BUFF_SIZE];
                int byteGet;
                for(int i = 0; i < BUFF_SIZE; i++) { buffer[i] = '\0'; }
                while(true)
                {
                    byteGet = recv(socketDesc, &buffer, sizeof(char) * BUFF_SIZE, 0);
                    logFile << "#INFO: recv bytes " << byteGet << " FLAG_SEND_STRING\n";
                    if(byteGet < 0) { logFile << "#ERROR: recv FLAG_SEND_STRING\n"; }
                    else if(byteGet == 0) break;

                    dataFromServer = dataFromServer + std::string(buffer);
                    if(byteGet < int(sizeof(char) * BUFF_SIZE)) break;
                }
            }
            else
            {
                logFile << "#INFO: thread listen_from_server recv wrong flag!\n";
                continue;
            }
            w->sendMessage(FLAG_UPDATE_FROM_SERV);
        }
     }
    logFile << "#INFO: thread listen_from_server stopped\n";
}

void send_to_server(int flag, int pos, char chr)
{
    MESSAGE_INFO msg;
    char bufferMSG[PACKETSIZE];
    int byteSend;
    int length;

    readyM_CV = false;
    std::lock_guard<std::mutex> lk(myMutex);
    sendDATA = true;
    msg.flag = flag;
    msg.posX = pos;
    msg.chr = chr;
    serialize_msg(&msg, bufferMSG);
    length = sizeof(bufferMSG)/sizeof(bufferMSG[0]);
    char *ptr = (char*) bufferMSG;
    while(length > 0)
    {
        byteSend = send(socketDesc, ptr, length, 0);
        if(byteSend < 0)
        {
            logFile << "#ERROR: send_to_server send\n";
            //close(socketDesc);
            //return -3;
        }
        ptr += byteSend;
        length -= byteSend;
    }
    readyM_CV = true;
    myConditionVariable.notify_all();
}

void send_to_server(std::string toSend)
{
    int length = toSend.length();
    char *buffer = new char[length];
    for(int k = 0; k < length; k++) { buffer[k] = toSend[k]; }
    char *ptr = buffer;
    int i;

    while (length > 0)
    {
        i = send(socketDesc, ptr, length, 0);
        if (i < 0)
        {
            logFile << "#ERROR: send_to_server toSend\n";
            break;
        }
        ptr += i;
        length -= i;
    }
    delete [] buffer;
}
