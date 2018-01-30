#include "include/main.hpp"

volatile bool endProgram = false;
int nSocketDesc;
vector < int > clientsDescriptors;
int numberClientsDescriptors = 0;
bool numberClientsDescriptorsChang = false;
pollfd *pollfdClientStruct = NULL;
mutex mtx;
condition_variable cv;
volatile bool ready = false;

void signal_callback_handler(int signum)
{
    cout << "#DEBUG: Signum = " << signum <<endl;
    endProgram = true;
}

void pollfd_array_resize()
{
    cout << "#DEBUG-control_client: pollfd_array_resize" << endl;
    if(pollfdClientStruct != NULL)
    {
        delete [] pollfdClientStruct;
        pollfdClientStruct = NULL;
    }
    cout << "#DEBUG-control_client: numberClientsDescriptors = " << numberClientsDescriptors << endl;
    if(numberClientsDescriptors != 0)
    {
        pollfdClientStruct = new pollfd[numberClientsDescriptors];
        for(size_t i = 0; i < clientsDescriptors.size(); i++)
        {
            pollfdClientStruct[i].fd = clientsDescriptors[i];
            pollfdClientStruct[i].events = POLLIN;
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

        if(numberClientsDescriptorsChang == true)
        {
            numberClientsDescriptorsChang = false;
            pollfd_array_resize();
        }

        readypoll = poll(pollfdClientStruct, numberClientsDescriptors, 10000);
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
                if(pollfdClientStruct[i].revents & POLLIN)
                {
                    dataSizeSendORRecv = recv_all(pollfdClientStruct[i].fd, bufferMSG);
                    if(dataSizeSendORRecv == RECIVE_ERROR)
                    {
                        cout <<"#DEBUG: While recv from descriptor " << pollfdClientStruct[i].fd << " get error." << endl;
                        cout << strerror(errno) << " :: " << errno << endl;
                    }
                    else if(dataSizeSendORRecv == RECIVE_ZERO)
                    {
                        cout <<"#DEBUG: Client with descriptor " << pollfdClientStruct[i].fd << " closed the connection." << endl;
                        close(pollfdClientStruct[i].fd);
                        canRemoveDesc = true;
                        --numberClientsDescriptors_temp;
                        clientsDescriptors.erase(std::remove(clientsDescriptors.begin(), clientsDescriptors.end(), pollfdClientStruct[i].fd), clientsDescriptors.end());
                    }
                    else
                    {
                        deserialize_msg(bufferMSG, &msgInfo);
                        if(msgInfo.flag == FLAG_INSERT_BEFORE)
                        {
                            ;
                        }
                        else if(msgInfo.flag == FLAG_REPLACE)
                        {
                            ;
                        }
                        else
                        {
                            cout <<"#DEBUG: Recive wrong flag " << msgInfo.flag << endl;
                            continue;
                        }
                        serialize_msg(&msgInfo, bufferMSG);
                        for(int cli = 0; cli < numberClientsDescriptors; cli++)
                        {
                            dataSizeSendORRecv = send_all(pollfdClientStruct[cli].fd, bufferMSG, sizeof(bufferMSG)/sizeof(bufferMSG[0]));
                            if(dataSizeSendORRecv == SEND_ERROR)
                            {
                                cout << "#DEBUG: Send error" << endl;
                                cout << strerror(errno) << " :: " << errno << endl;
                            }
                            else if(dataSizeSendORRecv == SEND_ALL_DATA)
                                cout << "#DEBUG: Data send" << endl;
                        }
                    }
                }
            }
            if(canRemoveDesc)
            {
                numberClientsDescriptors = numberClientsDescriptors_temp;
                numberClientsDescriptorsChang = true;
            }
            std::unique_lock<std::mutex> lck(mtx);
            ready = true;
            cv.notify_all();
        }
    }

    if(pollfdClientStruct != NULL)
    {
        for(int i = 0; i < numberClientsDescriptors; i++)
            close(pollfdClientStruct[i].fd);
        delete [] pollfdClientStruct;
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
            clientsDescriptors.push_back(nClientDesc);
            ++numberClientsDescriptors;
            numberClientsDescriptorsChang = true;
            unique_lock<std::mutex> lck(mtx);
            while (!ready) cv.wait(lck);
            //TODO: send strings
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
    thread controlClientThread(control_client);
    while(accept_clients());
    controlClientThread.join();
    close(nSocketDesc);
    clientsDescriptors.clear();
    cout << "#DEBUG: @@@@ SERVER IS SUCCESSIVELY CLOSED @@@@" << endl;
    return 0;
}
