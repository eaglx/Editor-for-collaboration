#include "main.hpp"

vector <clientACA> clientsDescriptorsACA;
bool numberClientsDescriptorsChangACA = false;
int numberClientsDescriptorsACA = 0;
bool manage_thread_ACA = false;
condition_variable cvACA;
mutex cv_mACA;
bool READY_THREAD_GLOBAL_SYNC_ACA = false;
pollfd *waitforACA = NULL;


void accept_connections_activ()
{
    int nClientDesc;
    int acvSocketDesc;
    int nBind, nListen;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t sockAddrSize;
    int nFoo = 1;
    int bytesSR;
    int userSpecialID;

    sockAddrSize = sizeof(struct sockaddr);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT_A);

    acvSocketDesc = socket(AF_INET, SOCK_STREAM, 0);
    if(acvSocketDesc < 0)
    {
        cout << "#ERROR-accept_connections_activ: Can't create a socket!!!" << endl;
        exit(-1);
    }
    setsockopt(acvSocketDesc, SOL_SOCKET, SO_REUSEADDR, (char *) &nFoo, sizeof(nFoo));

    fcntl(acvSocketDesc, F_SETFL, O_NONBLOCK);

    nBind = bind(acvSocketDesc, (struct sockaddr *) &serverAddr, sizeof(struct sockaddr));
    if(nBind < 0)
    {
        cout << "#ERROR-accept_connections_activ: Can't bind a socket!!!" << endl;
        exit(-2);
    }

    nListen = listen(acvSocketDesc, QUEUE_SIZE);
    if(nListen < 0)
    {
        cout << "#ERROR-accept_connections_activ: Can't set listen queue!!!" << endl;
        exit(-3);
    }

    cout << "#DEBUG-accept_connections_activ: accept_connections work." << endl;

    thread ccACAth(control_clientACA);

    while(!end_program)
    {
        nClientDesc = accept4(acvSocketDesc, (struct sockaddr *) &clientAddr, &sockAddrSize, SOCK_CLOEXEC);
        if(nClientDesc <= 0) continue;
        cout << "#DEBUG-accept_connections_activ: nClientDesc -> " << nClientDesc << endl;
        cout << "#DEBUG-accept_connections_activ: Client -> " << inet_ntoa((struct in_addr) clientAddr.sin_addr) << endl;

        bytesSR = recv(nClientDesc, &userSpecialID, sizeof(userSpecialID), 0);
        cout << "#DEBUG-accept_connections_activ: recv bytes " << bytesSR << endl;

        READY_THREAD_GLOBAL_SYNC_ACA = false;
        this_thread::sleep_for(std::chrono::seconds(1));
        lock_guard<std::mutex> lk2(cv_mACA);

        char c;
        ssize_t x;
        for(unsigned int i = 0; i < clientsDescriptorsACA.size(); i++)
        {
            x = recv(clientsDescriptorsACA[i].desc, &c, 1, MSG_PEEK);
            if (x > 0)
            {
                /* ...have data, leave it in socket buffer */
                cout << "#DEBUG-accept_connections_activ: Client with id exist " << clientsDescriptorsACA[i].id  << endl;
            }
            else if (x == 0)
            {
                /* ...handle FIN from client */
                cout << "#DEBUG-accept_connections_activ: Close client with id - FIN" << clientsDescriptorsACA[i].id  << endl;
                close(clientsDescriptorsACA[i].desc);
                clientsDescriptorsACA.erase(clientsDescriptorsACA.begin() + i);
                i = 0;
                --numberClientsDescriptorsACA;
                numberClientsDescriptorsChangACA = true;
            }
            else
            {
                 /* ...handle errors */
                cout << "#DEBUG-accept_connections_activ: Close client with id - error" << clientsDescriptorsACA[i].id  << endl;
                close(clientsDescriptorsACA[i].desc);
                clientsDescriptorsACA.erase(clientsDescriptorsACA.begin() + i);
                i = 0;
                --numberClientsDescriptorsACA;
                numberClientsDescriptorsChangACA = true;
            }
        }

        clientsDescriptorsACA.push_back(clientACA());
        clientsDescriptorsACA[clientsDescriptorsACA.size() - 1].desc = nClientDesc;
        clientsDescriptorsACA[clientsDescriptorsACA.size() - 1].id = userSpecialID;
        clientsDescriptorsACA[clientsDescriptorsACA.size() - 1].selectStart = 0;
        clientsDescriptorsACA[clientsDescriptorsACA.size() - 1].selectEnd = 0;
        ++numberClientsDescriptorsACA;
        numberClientsDescriptorsChangACA = true;

        cvACA.notify_all();
        READY_THREAD_GLOBAL_SYNC_ACA = true;
    }
    close(nSocketDesc);

    manage_thread_ACA = true;
    ccACAth.join();

    if(waitforACA != NULL) delete waitforACA;

    cout << "#DEBUG-accept_connections_activ: Closed" << endl;
}
