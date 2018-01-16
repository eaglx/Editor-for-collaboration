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

    thread tcth(test_connectionACA);
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

        clientsDescriptorsACA.push_back(clientACA());
        clientsDescriptorsACA[numberClientsDescriptorsACA].desc = nClientDesc;
        clientsDescriptorsACA[numberClientsDescriptorsACA].id = userSpecialID;
        ++numberClientsDescriptorsACA;
        numberClientsDescriptorsChangACA = true;

        cvACA.notify_all();
        READY_THREAD_GLOBAL_SYNC_ACA = true;
    }
    close(nSocketDesc);

    manage_thread_ACA = true;
    tcth.join();
    ccACAth.join();

    if(waitforACA != NULL) delete waitforACA;

    cout << "#DEBUG-accept_connections_activ: Closed" << endl;
}
