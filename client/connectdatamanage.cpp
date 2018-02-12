#include "connectdatamanage.h"

int recv_all(int socket, void *buffer, size_t length)
{
    char *ptr = (char*) buffer;
    int i;
    int iteration = 0;
    bool loopFinish = false;

    while (!loopFinish)
    {
        i = recv(socket, ptr, length, 0);
        if (i == -1) return -1;
        else if(i == 0)
        {
            if(iteration == 0) return 0;
            else loopFinish = true;
        }
        ptr += i;
        length -= i;
        ++iteration;
        if(length <= 0) loopFinish = true;
    }

    return i;
}

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
                    break;
                    errorRecv = true;
                }
                else if(byteGet == 0) { break; errorRecv = true; }
                length -= byteGet;
                if(length == 0) break;
            }
            if(errorRecv) { errorRecv = false; continue; }
            deserialize_msg(bufferMSG, &msg);
            if(msg.flag == FLAG_REPLACE)
            {
                dataFromServer[msg.posX] =  msg.chr;
            }
            else if(msg.flag == FLAG_REPLACE_CHARS)
            {
                char *buff_tm = new char[msg.length];
                recv_all(socketDesc, buff_tm, msg.length);
                std::string tm = "";
                for(int o = 0; o < msg.length; o++)
                {
                    tm = tm + buff_tm[o];
                }
                dataFromServer.replace(msg.posX, msg.length, tm);
            }
            else if(msg.flag == FLAG_APPEND)
            {
                dataFromServer.append(std::string(1, msg.chr));
            }
            else if(msg.flag == FLAG_APPEND_CHARS)
            {
                char *buff_tm = new char[msg.length];
                recv_all(socketDesc, buff_tm, msg.length);
                std::string tm = "";
                for(int o = 0; o < msg.length; o++)
                {
                    tm = tm + buff_tm[o];
                }
                dataFromServer.append(tm);
            }
            else if(msg.flag == FLAG_RM)
            {
                if(dataFromServer.length() > 0)
                    dataFromServer = dataFromServer.substr(0, (dataFromServer.size() - msg.posX));
            }
            else if(msg.flag == FLAG_DEL_ALL)
            {
                dataFromServer.clear();
                dataFromServer = "";
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

void send_to_server(int flag, int pos, int lng, char chr, std::string toSend)
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
    msg.length = lng;
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
        }
        ptr += byteSend;
        length -= byteSend;
    }

    if(lng != 0)
    {
        char *buf_tmp = new char[lng];
        for(int bt = 0; bt < lng; bt++)
        {
            buf_tmp[bt] = toSend[bt];
        }
        ptr = buf_tmp;
        while(lng > 0)
        {
            byteSend = send(socketDesc, ptr, lng, 0);
            if(byteSend < 0)
            {
                logFile << "#ERROR: send_to_server send\n";
            }
            ptr += byteSend;
            lng -= byteSend;
        }
        delete [] buf_tmp;
    }

    readyM_CV = true;
    myConditionVariable.notify_all();
}
