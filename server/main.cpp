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
            readypoll = poll(waitfor, numberClientsDescriptors, -1);
            if(readypoll == -1)
            {
                cout << "#DEBUG: control_client POLL ERROR" << endl;
                continue;
            }
            else if(readypoll == 0)
            {
                cout <<"#DEBUG: control_client POLL TIMEOUT" << endl;
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
                            close(waitfor[i].fd);
                            --numberClientsDescriptors;
                            clientsDescriptors.erase(clientsDescriptors.begin() + i);
                            numberClientsDescriptorsChang = true;

                            for(int i = 0; i < CLIENT_LIMIT; i++)
                                if(CST[i].descriptor == waitfor[i].fd)
                                {
                                    CST[i].descriptor = -1;
                                    CST[i].selectStart = 0;
                                    CST[i].selectEnd = 0;
                                    CST[i].allupdate = false;
                                    i = 100;
                                }

                            cout << "#DEBUG: control_client Delete client id - " << waitfor[i].fd << endl;
                        }
                        else
                        {
                            try
                            {
                                if(!manage_client(waitfor[i].fd, codeMsg))
                                {
                                    close(waitfor[i].fd);
                                    --numberClientsDescriptors;
                                    clientsDescriptors.erase(clientsDescriptors.begin() + i);
                                    numberClientsDescriptorsChang = true;

                                    for(int i = 0; i < CLIENT_LIMIT; i++)
                                        if(CST[i].descriptor == waitfor[i].fd)
                                        {
                                            CST[i].descriptor = -1;
                                            CST[i].selectStart = 0;
                                            CST[i].selectEnd = 0;
                                            CST[i].allupdate = false;
                                            i = 100;
                                        }

                                    cout << "#DEBUG: control_client Delete client id - " << waitfor[i].fd << endl;

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
    thread cth(control_client);

    int nClientDesc;
    int nBind, nListen;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t sockAddrSize;
    int nFoo = 1;

    id = msgget(123456, 0644|IPC_CREAT);
    if(id == -1)
    {
        cout << "#ERROR: Cannot create IPC!!!" << endl;
        return 1;
    }


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
        nClientDesc = accept(nSocketDesc, (struct sockaddr *) &clientAddr, &sockAddrSize);

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
                    i = 100;
                }
        }
    }

    cth.join();
    clientsDescriptors.clear();
    close(nSocketDesc);
    msgctl(id, IPC_RMID, NULL);
    return 0;
}

int main()
{
    signal(SIGINT, signal_callback_handler);

    for(int i = 0; i < CLIENT_LIMIT; i++)
    {
        CST[i].descriptor = -1;
        CST[i].selectStart = 0;
        CST[i].selectEnd = 0;
        CST[i].allupdate = false;
    }

    thread th_1(feditor);

    while(server());

    cout << "#DEBUG: Server is closed" << endl;
    th_1.join();

    return 0;
}
