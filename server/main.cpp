#include "main.hpp"

int id;
bool end_program = false;
int nSocketDesc;

condition_variable cv;
mutex cv_m;
bool ready = false;

vector < int > clientsDescriptors;
int numberClientsDescriptors = 0;
bool numberClientsDescriptorsChang = false;
pollfd *waitfor = NULL;

struct ClientSelectText CST[CLIENT_LIMIT];

void signal_callback_handler(int signum)
{
  cout << "#DEBUG: Signum = " << signum <<endl;
  end_program = true;
  cout << "#DEBUG: Start shutdown server" << endl;
}

void signal_callback_handler_PIPE(int signum)
{
    cout << "#ERROR: caught signal SIGPIPE " << signum << "!!!!!!" << endl;
}


void pollfd_array_resize()
{
    if(waitfor != NULL)
    {
        delete waitfor;
        waitfor = NULL;
    }
    waitfor = new pollfd[numberClientsDescriptors];
    for(size_t i = 0; i < clientsDescriptors.size(); i++)
    {
        waitfor[i].fd = clientsDescriptors[i];
        waitfor[i].events = POLLIN;
    }
}

void control_client()
{
    cout << "#DEBUG: control_client lounched" << endl;

    int readypoll;
    int codeMsg;

    while(!end_program)
    {
        if(numberClientsDescriptors == 0) continue;

        if(numberClientsDescriptorsChang == true)
        {
            numberClientsDescriptorsChang = false;
            pollfd_array_resize();
        }

        if(numberClientsDescriptors != 0)
        {
            readypoll = poll(waitfor, numberClientsDescriptors, 2000);
            if(readypoll == -1)
            {
                cout << "#DEBUG: control_client POLL ERROR" << endl;
                continue;
            }
            else if(readypoll == 0)
            {
                cout <<"#DEBUG: control_client POLL TIMEOUT" << endl;
                for(unsigned int i = 0; i < clientsDescriptors.size(); i++)
                {
                    cout <<"#DEBUG: Test connection to port " << waitfor[i].fd << endl;
                    codeMsg = send(waitfor[i].fd, &codeMsg, sizeof(codeMsg), MSG_NOSIGNAL);
                    if ((codeMsg == -1) || (CST[i].timeoutcount == 5))
                    {
                        --numberClientsDescriptors;
                        clientsDescriptors.erase(clientsDescriptors.begin() + i);
                        numberClientsDescriptorsChang = true;

                        cout << "#DEBUG: control_client Delete client id - " << waitfor[i].fd << endl;
                        for(int i = 0; i < CLIENT_LIMIT; i++)
                            if(CST[i].descriptor == waitfor[i].fd)
                            {
                                close(waitfor[i].fd);
                                CST[i].descriptor = -1;
                                CST[i].selectStart = 0;
                                CST[i].selectEnd = 0;
                                CST[i].allupdate = false;
                                CST[i].timeoutcount = 0;
                                i = 100;
                            }
                        i = clientsDescriptors.size();
                    }
                    else cout << "#DEBUG: Test connection to port OK" << endl;
                    ++CST[i].timeoutcount;
                }
                continue;
            }
            else
            {
                if(waitfor != NULL)
                for(int i = 0; i < numberClientsDescriptors; i++)
                    if(waitfor[i].revents & POLLIN)
                    {
                        if(read(waitfor[i].fd, &codeMsg, sizeof(codeMsg)) <= 0)
                        {
                            --numberClientsDescriptors;
                            clientsDescriptors.erase(clientsDescriptors.begin() + i);
                            numberClientsDescriptorsChang = true;

                            cout << "#DEBUG: control_client Delete client id - " << waitfor[i].fd << endl;
                            for(int i = 0; i < CLIENT_LIMIT; i++)
                                if(CST[i].descriptor == waitfor[i].fd)
                                {
                                    close(waitfor[i].fd);
                                    CST[i].descriptor = -1;
                                    CST[i].selectStart = 0;
                                    CST[i].selectEnd = 0;
                                    CST[i].allupdate = false;
                                    CST[i].timeoutcount = 0;
                                    i = 100;
                                }
                        }
                        else
                        {
                            try
                            {
                                if(!manage_client(waitfor[i].fd, codeMsg))
                                {
                                    --numberClientsDescriptors;
                                    clientsDescriptors.erase(clientsDescriptors.begin() + i);
                                    numberClientsDescriptorsChang = true;

                                    cout << "#DEBUG: control_client Delete client id - " << waitfor[i].fd << endl;
                                    for(int i = 0; i < CLIENT_LIMIT; i++)
                                        if(CST[i].descriptor == waitfor[i].fd)
                                        {
                                            close(waitfor[i].fd);
                                            CST[i].descriptor = -1;
                                            CST[i].selectStart = 0;
                                            CST[i].selectEnd = 0;
                                            CST[i].allupdate = false;
                                            CST[i].timeoutcount = 0;
                                            i = 100;
                                        }
                                }
                            }
                            catch(int e)
                            {
                                cout << "#DEBUG: Exception Nr. " << e << '\n';
                                continue;
                            }
                        }
                    }
           }
        }
    }

    if(waitfor != NULL)
    {
        for(int i = 0; i < numberClientsDescriptors; i++) close(waitfor[i].fd);
        delete waitfor;
    }
}

int server()
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

    cout << "#DEBUG: Sever work." << endl;

    while(!end_program)
    {
        nClientDesc = accept4(nSocketDesc, (struct sockaddr *) &clientAddr, &sockAddrSize, SOCK_CLOEXEC);

        if(nClientDesc <= 0) continue;

        cout << "#DEBUG: nClientDesc -> " << nClientDesc << endl;
        cout << "#DEBUG: Client -> " << inet_ntoa((struct in_addr) clientAddr.sin_addr) << endl;

        if(end_program)
            break;

        if(numberClientsDescriptors >= CLIENT_LIMIT)  // ONLY CLIENT_LIMIT users
            close(nClientDesc);
        else
        {
            ++numberClientsDescriptors;
            clientsDescriptors.push_back(nClientDesc);
            numberClientsDescriptorsChang = true;

            for(int i = 0; i < CLIENT_LIMIT; i++)
                if(CST[i].descriptor == -1)
                {
                    CST[i].descriptor = nClientDesc;
                    CST[i].selectStart = 0;
                    CST[i].selectEnd = 0;
                    CST[i].allupdate = false;
                    CST[i].timeoutcount = 0;
                    i = 100;
                }
        }
    }

    clientsDescriptors.clear();
    close(nSocketDesc);
    return 0;
}

int main()
{
    signal(SIGINT, signal_callback_handler);
    signal(SIGPIPE, signal_callback_handler_PIPE);
    //signal(SIGPIPE, SIG_IGN);

    id = msgget(123456, 0644|IPC_CREAT);
    if(id == -1)
    {
        cout << "#ERROR: Cannot create IPC!!!" << endl;
        return 1;
    }

    for(int i = 0; i < CLIENT_LIMIT; i++)
    {
        CST[i].descriptor = -1;
        CST[i].selectStart = 0;
        CST[i].selectEnd = 0;
        CST[i].allupdate = false;
        CST[i].timeoutcount = 0;
    }

    thread th_1(feditor);
    thread cth(control_client);

    while(server()) cout << "#INFO: Server exit from loop!!!!!!!!!!!!!!!" << endl;;

    cout << "#DEBUG: Server is closed" << endl;
    msgctl(id, IPC_RMID, NULL);
    th_1.join();
    cth.join();

    return 0;
}
