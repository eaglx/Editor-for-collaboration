#include "include/main.hpp"

volatile bool endProgram = false;
int nSocketDesc;
vector < int > clientsDescriptors;
int numberClientsDescriptors = 0;
bool isClientsDescriptorsChange = false;
pollfd *ClientStruct = NULL;
mutex myMutex;
condition_variable myCV;
volatile bool ready = true;
vector<string> fileBufferLines;

void signal_callback_handler(int signum)
{
    cout << "#DEBUG: Signum = " << signum <<endl;
    endProgram = true;
}

void string_resize(int x) { while((x + 1) > int(fileBufferLines.size())) { string s = ""; fileBufferLines.push_back(s); } }

void pollfd_array_resize()
{
    cout << "#DEBUG-control_client: pollfd_array_resize" << endl;
    if(ClientStruct != NULL)
    {
        delete [] ClientStruct;
        ClientStruct = NULL;
    }
    cout << "#DEBUG-control_client: numberClientsDescriptors = " << numberClientsDescriptors << endl;
    if(numberClientsDescriptors != 0)
    {
        ClientStruct = new pollfd[numberClientsDescriptors];
        for(size_t i = 0; i < clientsDescriptors.size(); i++)
        {
            ClientStruct[i].fd = clientsDescriptors[i];
            ClientStruct[i].events = POLLIN;
            cout << "#DEBUG-control_client: Active descriptions " << clientsDescriptors[i] << endl;
        }
    }
}

void control_client()
{
    int readypoll;
    int numberClientsDescriptors_temp;
    int dataSizeSendORRecv;
    bool canRemoveDesc;
    MESSAGE_INFO msgInfo;
    char bufferMSG[PACKETSIZE];

    cout << "#DEBUG: control_client lounched" << endl;
    while(!endProgram)
    {
        if(numberClientsDescriptors == 0)
        {
            this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }

        if(isClientsDescriptorsChange == true)
        {
            isClientsDescriptorsChange = false;
            pollfd_array_resize();
        }

        readypoll = poll(ClientStruct, numberClientsDescriptors, 10000);
        if(readypoll == -1)
        {
            cout << "#DEBUG: control_client POLL ERROR" << endl;
            //TODO: do something or not?
            continue;
        }
        else if(readypoll == 0)
        {
            cout <<"#DEBUG: control_client POLL TIMEOUT" << endl;
            //TODO: do something or not?
        }
        else
        {
            ready = false;
            canRemoveDesc = false;
            numberClientsDescriptors_temp = numberClientsDescriptors;
            for(int i = 0; i < numberClientsDescriptors; i++)
            {
                if(ClientStruct[i].revents & POLLIN)
                {
                    dataSizeSendORRecv = recv_all(ClientStruct[i].fd, bufferMSG, sizeof(bufferMSG)/sizeof(bufferMSG[0]));
                    if(dataSizeSendORRecv == RECIVE_ERROR)
                    {
                        cout <<"#DEBUG: While recv from descriptor " << ClientStruct[i].fd << " get error." << endl;
                        cout << strerror(errno) << " :: " << errno << endl;
                    }
                    else if(dataSizeSendORRecv == RECIVE_ZERO)
                    {
                        cout <<"#DEBUG: Client with descriptor " << ClientStruct[i].fd << " closed the connection." << endl;
                        close(ClientStruct[i].fd);
                        canRemoveDesc = true;
                        --numberClientsDescriptors_temp;
                        clientsDescriptors.erase(std::remove(clientsDescriptors.begin(), clientsDescriptors.end(), ClientStruct[i].fd), clientsDescriptors.end());
                    }
                    else
                    {
                        deserialize_msg(bufferMSG, &msgInfo);
                        if((msgInfo.posX + 1) > int(fileBufferLines.size())) { string_resize((msgInfo.posX + 1)); }
                        if(msgInfo.flag == FLAG_INSERT_BEFORE)
                        {
                            fileBufferLines[msgInfo.posX].insert(msgInfo.posY, string(1,msgInfo.chr));
                        }
                        else if(msgInfo.flag == FLAG_REPLACE)
                        {
                            fileBufferLines[msgInfo.posX].replace(msgInfo.posY, msgInfo.posY+1, string(1,msgInfo.chr));
                        }
                        else if(msgInfo.flag == FLAG_APPEND)
                        {
                            fileBufferLines[msgInfo.posX].append(string(1,msgInfo.chr));
                        }
                        else
                        {
                            cout <<"#DEBUG: Recive wrong flag " << msgInfo.flag << endl;
                            continue;
                        }
                        serialize_msg(&msgInfo, bufferMSG);
                        for(int cli = 0; cli < numberClientsDescriptors; cli++)
                        {
                            if(ClientStruct[cli].fd != ClientStruct[i].fd)
                            {
                                dataSizeSendORRecv = send_all(ClientStruct[cli].fd, bufferMSG, sizeof(bufferMSG)/sizeof(bufferMSG[0]));
                                if(dataSizeSendORRecv == SEND_ERROR)
                                {
                                    cout << "#DEBUG: Send error" << endl;
                                    cout << strerror(errno) << " :: " << errno << endl;
                                }
                                else if(dataSizeSendORRecv == SEND_ALL_DATA) { cout << "#DEBUG: Data send" << endl; }
                            }
                        }
                    }
                }
            }
            if(canRemoveDesc)
            {
                numberClientsDescriptors = numberClientsDescriptors_temp;
                isClientsDescriptorsChange = true;
            }
            std::unique_lock<std::mutex> lck(myMutex);
            ready = true;
            myCV.notify_all();
            cout << "**********************************" << endl;
            cout << "#DEBUG: SAVED DATA PRINT" << endl;
            for(unsigned int fbl = 0; fbl < fileBufferLines.size(); fbl++)
            {
                cout << fileBufferLines[fbl] << endl;
            }
            cout << "**********************************" << endl;
        }
    }

    if(ClientStruct != NULL)
    {
        for(int i = 0; i < numberClientsDescriptors; i++)
            close(ClientStruct[i].fd);
        delete [] ClientStruct;
    }
    cout << "#DEBUG: control_client closed" << endl;
}

int accept_clients()
{
    int nClientDesc;
    int nBind, nListen;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t sockAddrSize;
    int nFoo = 1;
    sockAddrSize = sizeof(struct sockaddr);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    string temp;
    char *buffer;

    nSocketDesc = socket(AF_INET, SOCK_STREAM, 0);
    if(nSocketDesc < 0)
    {
        cout << "#ERROR: Can't create a socket!!!" << endl;
        return -1;
    }
    setsockopt(nSocketDesc, SOL_SOCKET, SO_REUSEADDR, (char *) &nFoo, sizeof(nFoo));
    fcntl(nSocketDesc, F_SETFL, O_NONBLOCK);

    nBind = bind(nSocketDesc, (struct sockaddr *) &serverAddr, sizeof(struct sockaddr));
    if(nBind < 0)
    {
        cout << "#ERROR: Can't bind a socket!!!" << endl;
        return -2;
    }

    nListen = listen(nSocketDesc, QUEUE_SIZE);
    if(nListen < 0)
    {
        cout << "#ERROR: Can't set listen queue!!!" << endl;
        return -3;
    }

    cout << "#INFO: accept_clients start" << endl;
    while(!endProgram)
    {
        nClientDesc = accept4(nSocketDesc, (struct sockaddr *) &clientAddr, &sockAddrSize, SOCK_CLOEXEC);
        if(nClientDesc > 0)
        {
            cout << "#INFO: Client descriptor: " << nClientDesc << endl;
            cout << "#INFO: Client IP: " << inet_ntoa((struct in_addr) clientAddr.sin_addr) << endl;
            unique_lock<std::mutex> lck(myMutex);
            while (!ready) myCV.wait(lck);
            temp = "";
            for(unsigned int fbl = 0; fbl < fileBufferLines.size(); fbl++) { temp = temp + fileBufferLines[fbl]; temp = temp + "\n"; }
            buffer = new char[temp.size()];
            for(unsigned int k = 0; k < temp.size(); k++) { buffer[k] = temp[k]; }
            cout << "#DEBUG-accept_clients: Start send data with size " << temp.size() << endl;
            if(send_all(nClientDesc, buffer, temp.size()) < SEND_ERROR)
            {
                cout << "#DEBUG-accept_clients: Send error" << endl;
                delete [] buffer;
                continue;
            }
            cout << "#DEBUG-accept_clients:Finish send data" << endl;
            delete [] buffer;
            clientsDescriptors.push_back(nClientDesc);
            ++numberClientsDescriptors;
            isClientsDescriptorsChange = true;
        }
        this_thread::sleep_for(std::chrono::seconds(1));
    }

    cout << "#INFO: accept_clients stop" << endl;

    return 0;
}

int main()
{
    signal(SIGINT, signal_callback_handler);
    signal(SIGPIPE, SIG_IGN);

    cout << "#DEBUG: @@@@ SERVER STARTED @@@@" << endl;
    fileBufferLines.push_back(string(""));
    thread controlClientThread(control_client);
    while(accept_clients());
    controlClientThread.join();
    close(nSocketDesc);
    clientsDescriptors.clear();
    fileBufferLines.clear();
    cout << "#DEBUG: @@@@ SERVER IS SUCCESSIVELY CLOSED @@@@" << endl;
    return 0;
}
