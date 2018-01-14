#include "main.hpp"

int id;
bool end_program = false;
int nSocketDesc;

condition_variable cv;
mutex cv_m;
bool READY_THREAD_GLOBAL_SYNC = false;

vector < int > clientsDescriptors;
int numberClientsDescriptors = 0;
bool numberClientsDescriptorsChang = false;
pollfd *waitfor = NULL;

struct ClientSelectText CST[CLIENT_LIMIT];

void signal_callback_handler(int signum)
{
  cout << "#DEBUG: Signum = " << signum <<endl;
  end_program = true;
  READY_THREAD_GLOBAL_SYNC = true;
  cout << "#DEBUG: Start shutdown server" << endl;
}

void signal_callback_handler_PIPE(int signum)
{
    cout << "#ERROR: caught signal SIGPIPE " << signum << "!!!!!!" << endl;
    /*for(unsigned int i = 0; i < clientsDescriptors.size(); i++)
    {
        cout << "#DEBUG-schPIPE: Close descriptor - " << waitfor[i].fd << endl;
        close(waitfor[i].fd);
        waitfor[i].fd = -1;
    }*/
}


void pollfd_array_resize()
{
    cout << "#DEBUG: pollfd_array_resize" << endl;
    if(waitfor != NULL)
    {
        delete waitfor;
        waitfor = NULL;
    }
    cout << "#DEBUG: numberClientsDescriptors = " << numberClientsDescriptors << endl;
    if(numberClientsDescriptors != 0)
    {
        waitfor = new pollfd[numberClientsDescriptors];
        for(size_t i = 0; i < clientsDescriptors.size(); i++)
        {
            waitfor[i].fd = clientsDescriptors[i];
            waitfor[i].events = POLLIN;
            cout << "#DEBUG: Active descriptions " << clientsDescriptors[i] << endl;
        }
    }
}

void control_client()
{
    cout << "#DEBUG: control_client lounched" << endl;

    int readypoll;
    int codeMsg;

    while(!end_program)
    {
        unique_lock<std::mutex> lk(cv_m);
        //cerr << "#DEBUG: control_client WAITING... \n";
        cv.wait(lk, []{return READY_THREAD_GLOBAL_SYNC;});
        //cerr << "#DEBUG: control_client FINISHED WAITING.\n";

        if(numberClientsDescriptors == 0) continue;

        if(numberClientsDescriptorsChang == true)
        {
            numberClientsDescriptorsChang = false;
            pollfd_array_resize();
        }

        if(numberClientsDescriptors != 0)
        {
            readypoll = poll(waitfor, numberClientsDescriptors, 1000);
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
                    cout <<"#DEBUG: Test connection to descriptor " << waitfor[i].fd << endl;
                    codeMsg = send(waitfor[i].fd, &codeMsg, sizeof(codeMsg), MSG_NOSIGNAL);
                    if ((codeMsg == -1) || (CST[i].timeoutcount == 3))
                    {
                        --numberClientsDescriptors;
                        clientsDescriptors.erase(clientsDescriptors.begin() + i);
                        numberClientsDescriptorsChang = true;

                        cout << "#DEBUG: Delete due timeout client id " << waitfor[i].fd << endl;
                        for(int i = 0; i < CLIENT_LIMIT; i++)
                            if(CST[i].descriptor == waitfor[i].fd)
                            {
                                close(waitfor[i].fd);
                                CST[i].descriptor = -1;
                                CST[i].timeoutcount = 0;
                            }
                        i = clientsDescriptors.size();
                    }
                    else
                    {
                        cout << "#DEBUG: Test connection to port OK" << endl;
                        ++CST[i].timeoutcount;
                    }
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

                            cout << "#DEBUG: control_client Delete client id - " << waitfor[i].fd << endl;
                            for(int i = 0; i < CLIENT_LIMIT; i++)
                                if(CST[i].descriptor == waitfor[i].fd)
                                {
                                    close(waitfor[i].fd);
                                    CST[i].descriptor = -1;
                                    CST[i].timeoutcount = 0;
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
                                            CST[i].timeoutcount = 0;
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
    
    cout << "#DEBUG: control_client closed" << endl;
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
            READY_THREAD_GLOBAL_SYNC = false;
            this_thread::sleep_for(std::chrono::seconds(1));
            lock_guard<std::mutex> lk(cv_m);
            cerr << "#DEBUG: server manage new connection\n";

            int countCLIENT = 0;
            for(int i = 0; i < CLIENT_LIMIT; i++)
            {
                cout << "#DEBUG: This client desc is saved: " << CST[i].descriptor << endl;
                if(CST[i].descriptor == -1) ++countCLIENT;
                else
                {
                    cout <<"#DEBUG: Test connection to descriptor " << endl;
                    char c;
                    ssize_t x = recv(CST[i].descriptor, &c, 1, MSG_PEEK);
                    if (x > 0)
                    {
                        /* ...have data, leave it in socket buffer */
                        cout << "#DEBUG: This client exist: " << CST[i].descriptor << endl;
                    }
                    else if (x == 0)
                    {
                        /* ...handle FIN from client */
                        close(CST[i].descriptor);
                        CST[i].descriptor = -1;
                        ++countCLIENT;
                    }
                    else
                    {
                         /* ...handle errors */
                        close(CST[i].descriptor);
                        CST[i].descriptor = -1;
                        ++countCLIENT;
                    }
                }
            }

            if(countCLIENT == CLIENT_LIMIT)
            {
                clientsDescriptors.clear();
                numberClientsDescriptorsChang = true;
                numberClientsDescriptors = 0;
            }
            usleep(1000 * 1); //1 sec

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
            clientsDescriptors.push_back(nClientDesc);
            numberClientsDescriptorsChang = true;
            ++numberClientsDescriptors;

            cv.notify_all();
            READY_THREAD_GLOBAL_SYNC = true;
        }
    }

    clientsDescriptors.clear();
    close(nSocketDesc);
    cout << "#DEBUG: server closed" << endl;
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

    for(int i = 0; i < CLIENT_LIMIT; i++) CST[i].descriptor = -1;

    thread th_1(feditor);
    thread cth(control_client);

    while(server()) cout << "#INFO: Server exit from loop!!!!!!!!!!!!!!!" << endl;

    cout << "#DEBUG: Server is closed" << endl;
    msgctl(id, IPC_RMID, NULL);
    th_1.join();
    cth.join();

    cout << "#DEBUG: @@@@ EVERYTHING IS SUCCESSIVELY CLOSED @@@@" << endl;
    return 0;
}
