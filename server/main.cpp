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
string fileBuffer;

void signal_callback_handler(int signum)
{
    int value;
    cout << "#DEBUG: Signum = " << signum <<endl;
    while(true)
    {
        cout << "#SERVER: What to do?" << endl;
        cout << "1 - close server\n2 - clear cache" << endl;
        cout << "Choose: ";
        cin >> value;
        if(value == 1) { endProgram = true; break; }
        else if(value == 2) { fileBuffer.clear(); fileBuffer = ""; break; }
        else { break; }
    }
}

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
            cout << "#DEBUG-control_client: Active descriptor " << clientsDescriptors[i] << endl;
        }
    }
}

void delete_DEAD_client(int sockCli, bool &canRemoveDesc, int &numberClientsDescriptors_temp)
{
    cout << "#DEBUG: Client with descriptor " << sockCli << " closed the connection." << endl;
    close(sockCli);
    canRemoveDesc = true;
    --numberClientsDescriptors_temp;
    clientsDescriptors.erase(std::remove(clientsDescriptors.begin(), clientsDescriptors.end(), sockCli), clientsDescriptors.end());
}

void error_read_client(int sockCli)
{
    cout << "#DEBUG: Send error to " << sockCli << endl;
    cout << strerror(errno) << " :: " << errno << endl;
}

void control_client()
{
    int readypoll;
    int numberClientsDescriptors_temp;
    int dataSizeSendORRecv;
    bool canRemoveDesc;
    MESSAGE_INFO msgInfo;
    char bufferMSG[PACKETSIZE];
    ofstream myfile;
    char *buff_tm;

    cout << "#DEBUG: control_client lounched" << endl;
    while(!endProgram)
    {
        if(numberClientsDescriptors == 0) { continue; }

        if(isClientsDescriptorsChange == true)
        {
            isClientsDescriptorsChange = false;
            pollfd_array_resize();
        }

        #define POLL_TIMEOUT 10000
        readypoll = poll(ClientStruct, numberClientsDescriptors, POLL_TIMEOUT);
        if(readypoll == -1)
        {
            cout << "#DEBUG: control_client POLL ERROR" << endl;
            //TODO: do something or not?
            continue;
        }
        else if(readypoll == 0)
        {
            //cout <<"#DEBUG: control_client POLL TIMEOUT" << endl;
            //TODO: do something or not?
            continue;
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
                        error_read_client(ClientStruct[i].fd);
                        if(errno == ECONNRESET)
                        {
                            cout << "#INFO: ECONNRESET" << endl;
                        }
                        else if(errno == EPIPE)
                        {
                            cout << "#INFO: EPIPE" << endl;
                        }
                    }
                    else if(dataSizeSendORRecv == RECIVE_ZERO)
                    {
                        delete_DEAD_client(ClientStruct[i].fd, canRemoveDesc, numberClientsDescriptors_temp);
                    }
                    else
                    {
                        deserialize_msg(bufferMSG, &msgInfo);
                        cout << "#DEBUG: Recive flag " << msgInfo.flag << " from " <<  ClientStruct[i].fd << endl;
                        if(msgInfo.flag == FLAG_INSERT_BEFORE)
                        {
                            fileBuffer.insert(msgInfo.posX, string(1, msgInfo.chr));
                        }
                        else if(msgInfo.flag == FLAG_REPLACE)
                        {
                            /*buff_tm = new char[msgInfo.length];
                            dataSizeSendORRecv = recv_all(ClientStruct[i].fd, buff_tm, msgInfo.length);
                            if(dataSizeSendORRecv == RECIVE_ERROR)
                            {
                                error_read_client(ClientStruct[i].fd);
                                delete [] buff_tm;
                                continue;
                            }
                            else if(dataSizeSendORRecv == RECIVE_ZERO)
                            {
                                delete_DEAD_client(ClientStruct[i].fd, canRemoveDesc, numberClientsDescriptors_temp);
                                delete [] buff_tm;
                                continue;
                            }

                            if(errno == ECONNRESET)
                            {
                                cout << "#INFO: ECONNRESET" << endl;
                            }*/
                            fileBuffer[msgInfo.posX] = msgInfo.chr;
                            // TODO
                        }
                        else if(msgInfo.flag == FLAG_APPEND)
                        {
                            /*buff_tm = new char[msgInfo.length];
                            dataSizeSendORRecv = recv_all(ClientStruct[i].fd, buff_tm, msgInfo.length);
                            if(dataSizeSendORRecv == RECIVE_ERROR)
                            {
                                error_read_client(ClientStruct[i].fd);
                                delete [] buff_tm;
                                continue;
                            }
                            else if(dataSizeSendORRecv == RECIVE_ZERO)
                            {
                                delete_DEAD_client(ClientStruct[i].fd, canRemoveDesc, numberClientsDescriptors_temp);
                                delete [] buff_tm;
                                continue;
                            }

                            if(errno == ECONNRESET)
                            {
                                cout << "#INFO: ECONNRESET" << endl;
                            }*/
                            fileBuffer.append(string(1, msgInfo.chr));
                            // TODO
                        }
                        else if(msgInfo.flag == FLAG_RM)
                        {
                            if(fileBuffer.length() > 0)
                                fileBuffer = fileBuffer.substr(0, (fileBuffer.size() - 1));
                        }
                        else if(msgInfo.flag == FLAG_DEL_ALL)
                        {
                            fileBuffer.clear();
                            fileBuffer = "";
                        }
                        else
                        {
                            cout << "#DEBUG: Recive wrong flag " << msgInfo.flag << endl;
                            continue;
                        }
                        serialize_msg(&msgInfo, bufferMSG);
                        for(int cli = 0; cli < numberClientsDescriptors; cli++)
                        {
                            dataSizeSendORRecv = send_all(ClientStruct[cli].fd, bufferMSG, sizeof(bufferMSG)/sizeof(bufferMSG[0]));
                            if(dataSizeSendORRecv == SEND_ERROR)
                            {
                                cout << "#DEBUG: Send error to " << ClientStruct[cli].fd << endl;
                                cout << strerror(errno) << " :: " << errno << endl;
                            }
                            else if(dataSizeSendORRecv == SEND_ALL_DATA) { cout << "#DEBUG: Data send to " << ClientStruct[cli].fd << endl; }

                            if(errno == ECONNRESET)
                            {
                                cout << "#INFO: ECONNRESET" << endl;
                                delete_DEAD_client(ClientStruct[i].fd, canRemoveDesc, numberClientsDescriptors_temp);
                            }
                            else if(errno == EPIPE)
                            {
                                cout << "#INFO: EPIPE" << endl;
                                delete_DEAD_client(ClientStruct[i].fd, canRemoveDesc, numberClientsDescriptors_temp);
                            }

                            /*if((msgInfo.flag == FLAG_REPLACE) || (msgInfo.flag == FLAG_APPEND))
                            {
                                dataSizeSendORRecv = send_all(ClientStruct[cli].fd, buff_tm, msgInfo.length);
                                if(dataSizeSendORRecv == SEND_ERROR)
                                {
                                    cout << "#DEBUG: Send raw string error to " << ClientStruct[cli].fd << endl;
                                    cout << strerror(errno) << " :: " << errno << endl;
                                }
                                else if(dataSizeSendORRecv == SEND_ALL_DATA) { cout << "#DEBUG: Data send raw string to " << ClientStruct[cli].fd << endl; }
                                delete [] buff_tm;
                            }*/
                        }
                        myfile.open("cache.dump");
                        myfile << fileBuffer;
                        myfile.close();
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
    socklen_t nFoo_size = sizeof(nFoo);
    sockAddrSize = sizeof(struct sockaddr);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    char *buffer;

    nSocketDesc = socket(AF_INET, SOCK_STREAM, 0);
    if(nSocketDesc < 0)
    {
        cout << "#ERROR: Can't create a socket!!!" << endl;
        return -1;
    }
    if(setsockopt(nSocketDesc, SOL_SOCKET, SO_REUSEADDR, (char *) &nFoo, sizeof(nFoo)) < 0)
    {
        cout << "#ERROR: Can't setsockopt_SO_REUSEADDR!!!" << endl;
        close(nSocketDesc);
        return -2;
    }
    if(setsockopt(nSocketDesc, SOL_SOCKET, SO_KEEPALIVE, (char *) &nFoo, sizeof(nFoo)) < 0)
    {
        cout << "#ERROR: Can't setsockopt_SO_KEEPALIVE!!!" << endl;
        close(nSocketDesc);
        return -2;
    }
    /* Set the number of seconds the connection must be idle before sending a KA probe. */
    nFoo = 20;
    if (setsockopt(nSocketDesc, IPPROTO_TCP, TCP_KEEPIDLE, &nFoo, sizeof(nFoo)) < 0)
    {
        cout << "#ERROR: Can't setsockopt_TCP_KEEPIDLE!!!" << endl;
        close(nSocketDesc);
        return -2;
    }

    /* Set how often in seconds to resend an unacked KA probe. */
    nFoo = 5;
    if (setsockopt(nSocketDesc, IPPROTO_TCP, TCP_KEEPINTVL, &nFoo, sizeof(nFoo)) < 0)
    {
        cout << "#ERROR: Can't setsockopt_TCP_KEEPINTVL!!!" << endl;
        close(nSocketDesc);
        return -2;
    }

    /* Set how many times to resend a KA probe if previous probe was unacked. */
    nFoo = 3;
    if (setsockopt(nSocketDesc, IPPROTO_TCP, TCP_KEEPCNT, &nFoo, sizeof(nFoo)) < 0)
    {
        cout << "#ERROR: Can't setsockopt_TCP_KEEPCNT!!!" << endl;
        close(nSocketDesc);
        return -2;
    }
    fcntl(nSocketDesc, F_SETFL, O_NONBLOCK);

    if(getsockopt(nSocketDesc, SOL_SOCKET, SO_KEEPALIVE, &nFoo, &nFoo_size) < 0)
    {
        cout << "#ERROR: Can't getsockopt!!!" << endl;
        return -2;
    }
    cout << "#INFO: SO_KEEPALIVE is " << (nFoo ? "ON" : "OFF") << endl;

    nBind = bind(nSocketDesc, (struct sockaddr *) &serverAddr, sizeof(struct sockaddr));
    if(nBind < 0)
    {
        cout << "#ERROR: Can't bind a socket!!!" << endl;
        return -3;
    }

    nListen = listen(nSocketDesc, QUEUE_SIZE);
    if(nListen < 0)
    {
        cout << "#ERROR: Can't set listen queue!!!" << endl;
        return -4;
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
            if(fileBuffer.length() == 0)
            {
                fileBuffer = "?/?/#";
            }
            buffer = new char[fileBuffer.size()];
            for(unsigned int k = 0; k < fileBuffer.size(); k++) { buffer[k] = fileBuffer[k]; }
            cout << "#DEBUG-accept_clients: Start send data with size " << fileBuffer.size() << endl;
            if(send_all(nClientDesc, buffer, fileBuffer.size()) == SEND_ERROR)
            {
                cout << "#DEBUG-accept_clients: Send error" << endl;
                delete [] buffer;
                close(nClientDesc);
                continue;
            }
            if(fileBuffer == "?/?/#") { fileBuffer = ""; }
            cout << "#DEBUG-accept_clients:Finish send data" << endl;
            delete [] buffer;
            clientsDescriptors.push_back(nClientDesc);
            ++numberClientsDescriptors;
            isClientsDescriptorsChange = true;
        }
    }

    cout << "#INFO: accept_clients stop" << endl;
    return 0;
}

int main()
{
    signal(SIGINT, signal_callback_handler);
    signal(SIGPIPE, SIG_IGN);

    cout << "#DEBUG: @@@@ SERVER STARTED @@@@" << endl;
    ifstream myfile("cache.dump");
    if (myfile.is_open())
    {
        string line;
        fileBuffer = "";
        while ( getline (myfile, line) )
        {
          fileBuffer = fileBuffer + line;
        }
        myfile.close();
    }
    else fileBuffer = "";
    thread controlClientThread(control_client);
    while(accept_clients());
    controlClientThread.join();
    close(nSocketDesc);
    clientsDescriptors.clear();
    fileBuffer.clear();
    cout << "#DEBUG: @@@@ SERVER IS SUCCESSIVELY CLOSED @@@@" << endl;
    remove("cache.dump");
    return 0;
}
