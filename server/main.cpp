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
    cout << "#DEBUG: Signum = " << signum <<endl;
    endProgram = true;
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

void control_client()
{
    int readypoll;
    int numberClientsDescriptors_temp;
    int dataSizeSendORRecv;
    bool canRemoveDesc;
    MESSAGE_INFO msgInfo;
    char bufferMSG[PACKETSIZE];
    ofstream myfile;

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
                        cout << "#DEBUG: While recv from descriptor " << ClientStruct[i].fd << " get error." << endl;
                        cout << strerror(errno) << " :: " << errno << endl;
                    }
                    else if(dataSizeSendORRecv == RECIVE_ZERO)
                    {
                        cout << "#DEBUG: Client with descriptor " << ClientStruct[i].fd << " closed the connection." << endl;
                        close(ClientStruct[i].fd);
                        canRemoveDesc = true;
                        --numberClientsDescriptors_temp;
                        clientsDescriptors.erase(std::remove(clientsDescriptors.begin(), clientsDescriptors.end(), ClientStruct[i].fd), clientsDescriptors.end());
                    }
                    else
                    {
                        deserialize_msg(bufferMSG, &msgInfo);
                        cout << "#DEBUG: Recive flag " << msgInfo.flag << " from " <<  ClientStruct[i].fd << endl;
                        if(msgInfo.flag == FLAG_INSERT_BEFORE)
                        {
                            //cout << "#DEBUG: FLAG_INSERT_BEFORE" << endl;
                            fileBuffer.insert(msgInfo.posX, string(1, msgInfo.chr));
                        }
                        else if(msgInfo.flag == FLAG_REPLACE)
                        {
                            //cout << "#DEBUG: FLAG_REPLACE" << endl;
                            while(unsigned(msgInfo.posX) > fileBuffer.size())
                                fileBuffer.resize(fileBuffer.size() + 1, ' ');
                            fileBuffer.replace(msgInfo.posX, msgInfo.posX+1, string(1, msgInfo.chr));
                        }
                        else if(msgInfo.flag == FLAG_APPEND)
                        {
                            //cout << "#DEBUG: FLAG_APPEND" << endl;
                            //if(fileBuffer.length() != unsigned(msgInfo.posX - 1))
                            //    cout << "#DEBUG: Append in wrong place " << endl;
                            fileBuffer.append(string(1, msgInfo.chr));
                        }
                        else if(msgInfo.flag == FLAG_RM)
                        {
                            //cout << "#DEBUG: FLAG_RM" << endl;
                            fileBuffer = fileBuffer.substr(0, (fileBuffer.size() - 1));
                            //if(fileBuffer.length() == 0)
                                //fileBuffer = "";
                        }
                        else if(msgInfo.flag == FLAG_DEL_ALL)
                        {
                            //cout << "#DEBUG: FLAG_DEL_ALL" << endl;
                            fileBuffer.clear();
                            fileBuffer = "";
                        }
                        else if(msgInfo.flag == FLAG_START_SELECTION)
                        {
                            //TODO
                        }
                        else if(msgInfo.flag == FLAG_END_SELECTION)
                        {
                            //TODO
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
            if(fileBuffer.length() == 0)
            {
                fileBuffer = "?/?/#";
            }
            buffer = new char[fileBuffer.size()];
            for(unsigned int k = 0; k < fileBuffer.size(); k++) { buffer[k] = fileBuffer[k]; }
            cout << "#DEBUG-accept_clients: Start send data with size " << fileBuffer.size() << endl;
            if(send_all(nClientDesc, buffer, fileBuffer.size()) < SEND_ERROR)
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
    else fileBuffer = "#Hey#";
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
