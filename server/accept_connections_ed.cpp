#include "main.hpp"

void accept_connections_ed()
{
    int nClientDesc;
    int nBind, nListen;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t sockAddrSize;
    int nFoo = 1;

    sockAddrSize = sizeof(struct sockaddr);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT_E);

    nSocketDesc = socket(AF_INET, SOCK_STREAM, 0);
    if(nSocketDesc < 0)
    {
        cout << "#ERROR-accept_connections_ed: Can't create a socket!!!" << endl;
        exit(-1);
    }
    setsockopt(nSocketDesc, SOL_SOCKET, SO_REUSEADDR, (char *) &nFoo, sizeof(nFoo));

    fcntl(nSocketDesc, F_SETFL, O_NONBLOCK);

    nBind = bind(nSocketDesc, (struct sockaddr *) &serverAddr, sizeof(struct sockaddr));
    if(nBind < 0)
    {
        cout << "#ERROR-accept_connections_ed: Can't bind a socket!!!" << endl;
        exit(-2);
    }

    nListen = listen(nSocketDesc, QUEUE_SIZE);
    if(nListen < 0)
    {
        cout << "#ERROR-accept_connections_ed: Can't set listen queue!!!" << endl;
        exit(-3);
    }

    cout << "#DEBUG-accept_connections_ed: accept_connections work." << endl;

    while(!end_program)
    {
        nClientDesc = accept4(nSocketDesc, (struct sockaddr *) &clientAddr, &sockAddrSize, SOCK_CLOEXEC);

        if(nClientDesc <= 0) continue;

        cout << "#DEBUG-accept_connections_ed: nClientDesc -> " << nClientDesc << endl;
        cout << "#DEBUG-accept_connections_ed: Client -> " << inet_ntoa((struct in_addr) clientAddr.sin_addr) << endl;

        if(end_program)
            break;

        if(numberClientsDescriptors >= CLIENT_LIMIT)  // ONLY CLIENT_LIMIT users
            close(nClientDesc);
        else
        {
            READY_THREAD_GLOBAL_SYNC = false;
            this_thread::sleep_for(std::chrono::seconds(1));
            lock_guard<std::mutex> lk(cv_m);
            cerr << "#DEBUG-accept_connections_ed: accept_connections manage new connection\n";

            int countCLIENT = 0;
            for(int i = 0; i < CLIENT_LIMIT; i++)
            {
                cout << "#DEBUG-accept_connections_ed: This client desc is saved: " << CST[i].descriptor << endl;
                if(CST[i].descriptor == -1) ++countCLIENT;
                else
                {
                    cout <<"#DEBUG-accept_connections_ed: Test connection to descriptor " << endl;
                    char c;
                    ssize_t x = recv(CST[i].descriptor, &c, 1, MSG_PEEK);
                    if (x > 0)
                    {
                        /* ...have data, leave it in socket buffer */
                        cout << "#DEBUG-accept_connections_ed: This client exist: " << CST[i].descriptor << endl;
                    }
                    else if (x == 0)
                    {
                        /* ...handle FIN from client */
                        close(CST[i].descriptor);
                        CST[i].descriptor = -1;
                        CST[i].clientSPECIAL_ID = -1;
                        ++countCLIENT;
                    }
                    else
                    {
                         /* ...handle errors */
                        close(CST[i].descriptor);
                        CST[i].descriptor = -1;
                        CST[i].clientSPECIAL_ID = -1;
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
                    recv(nClientDesc, &CST[i].clientSPECIAL_ID, sizeof(CST[i].clientSPECIAL_ID), 0);
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
    cout << "#DEBUG-accept_connections_ed: Closed" << endl;
    exit(0);
}
