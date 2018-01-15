#include "main.hpp"

char bufforFE[PAGE_X][PAGE_Y];

bool end_program = false;
int nSocketDesc;

condition_variable cv;
mutex cv_m;
bool READY_THREAD_GLOBAL_SYNC = false;

vector < int > clientsDescriptors;
int numberClientsDescriptors = 0;
bool numberClientsDescriptorsChang = false;

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
}

int accept_connections()
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
        cout << "#ERROR-accept_connections: Can't create a socket!!!" << endl;
        return -1;
    }
    setsockopt(nSocketDesc, SOL_SOCKET, SO_REUSEADDR, (char *) &nFoo, sizeof(nFoo));

    fcntl(nSocketDesc, F_SETFL, O_NONBLOCK);

    nBind = bind(nSocketDesc, (struct sockaddr *) &serverAddr, sizeof(struct sockaddr));
    if(nBind < 0)
    {
        cout << "#ERROR-accept_connections: Can't bind a socket!!!" << endl;
        return -2;
    }

    nListen = listen(nSocketDesc, QUEUE_SIZE);
    if(nListen < 0)
    {
        cout << "#ERROR-accept_connections: Can't set listen queue!!!" << endl;
        return -3;
    }

    cout << "#DEBUG-accept_connections: accept_connections work." << endl;

    while(!end_program)
    {
        nClientDesc = accept4(nSocketDesc, (struct sockaddr *) &clientAddr, &sockAddrSize, SOCK_CLOEXEC);

        if(nClientDesc <= 0) continue;

        cout << "#DEBUG-accept_connections: nClientDesc -> " << nClientDesc << endl;
        cout << "#DEBUG-accept_connections: Client -> " << inet_ntoa((struct in_addr) clientAddr.sin_addr) << endl;

        if(end_program)
            break;

        if(numberClientsDescriptors >= CLIENT_LIMIT)  // ONLY CLIENT_LIMIT users
            close(nClientDesc);
        else
        {
            READY_THREAD_GLOBAL_SYNC = false;
            this_thread::sleep_for(std::chrono::seconds(1));
            lock_guard<std::mutex> lk(cv_m);
            cerr << "#DEBUG-accept_connections: accept_connections manage new connection\n";

            int countCLIENT = 0;
            for(int i = 0; i < CLIENT_LIMIT; i++)
            {
                cout << "#DEBUG-accept_connections: This client desc is saved: " << CST[i].descriptor << endl;
                if(CST[i].descriptor == -1) ++countCLIENT;
                else
                {
                    cout <<"#DEBUG-accept_connections: Test connection to descriptor " << endl;
                    char c;
                    ssize_t x = recv(CST[i].descriptor, &c, 1, MSG_PEEK);
                    if (x > 0)
                    {
                        /* ...have data, leave it in socket buffer */
                        cout << "#DEBUG-accept_connections: This client exist: " << CST[i].descriptor << endl;
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
    cout << "#DEBUG-accept_connections: Closed" << endl;
    return 0;
}

int main()
{
    signal(SIGINT, signal_callback_handler);
    signal(SIGPIPE, signal_callback_handler_PIPE);
    //signal(SIGPIPE, SIG_IGN);

    for(int i = 0; i < PAGE_X; i++)
        for(int j = 0; j < PAGE_Y; j++)
            bufforFE[i][j] = '\0';

    for(int i = 0; i < CLIENT_LIMIT; i++) CST[i].descriptor = -1;

    thread cth(control_client);

    while(accept_connections());

    cth.join();

    cout << "#DEBUG: @@@@ EVERYTHING IS SUCCESSIVELY CLOSED @@@@" << endl;
    return 0;
}
