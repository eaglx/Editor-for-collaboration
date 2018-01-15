#include "main.hpp"

bool test_connection_th_off;
void test_connection_th(int client_desc_test)
{
    char c;
    ssize_t x;
    cout << "#DEBUG-accept_connections_activ: test_connection_th run" << endl;
    while(!test_connection_th_off)
    {
        x = recv(client_desc_test, &c, 1, MSG_PEEK);
        if (x > 0)
        {
            /* ...have data, leave it in socket buffer */
            cout << "#DEBUG-test_connection_th: Cclient exist" << endl;
        }
        else if (x == 0)
        {
            /* ...handle FIN from client */
            cout << "#DEBUG-test_connection_th: Close client - FIN" << endl;
            close(client_desc_test);
            test_connection_th_off = true;
        }
        else
        {
             /* ...handle errors */
             cout << "#DEBUG-test_connection_th: Close client - error" << endl;
            close(client_desc_test);
            test_connection_th_off = true;
        }
    }
    cout << "#DEBUG-accept_connections_activ: test_connection_th stop" << endl;
}

void accept_connections_activ()
{
    int nClientDesc;
    int acvSocketDesc;
    int nBind, nListen;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t sockAddrSize;
    int nFoo = 1;
    int code_msg;
    int bytesSR;

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

    while(!end_program)
    {
        nClientDesc = accept4(acvSocketDesc, (struct sockaddr *) &clientAddr, &sockAddrSize, SOCK_CLOEXEC);
        if(nClientDesc <= 0) continue;
        cout << "#DEBUG-accept_connections_activ: nClientDesc -> " << nClientDesc << endl;
        cout << "#DEBUG-accept_connections_activ: Client -> " << inet_ntoa((struct in_addr) clientAddr.sin_addr) << endl;

        test_connection_th_off = false;
        thread th(test_connection_th, nClientDesc);

        bytesSR = recv(nClientDesc, &code_msg, sizeof(code_msg), 0);
        cout << "#DEBUG-accept_connections_activ: recv bytes " << bytesSR << " code_msg " << code_msg << endl;
        client_handle_activ(nClientDesc, code_msg);
        bytesSR = recv(nClientDesc, &code_msg, sizeof(code_msg), 0);
        cout << "#DEBUG-accept_connections_activ: recv bytes " << bytesSR << " code_msg " << code_msg << endl;
        client_handle_activ(nClientDesc, code_msg);
        bytesSR = recv(nClientDesc, &code_msg, sizeof(code_msg), 0);
        cout << "#DEBUG-accept_connections_activ: recv bytes " << bytesSR << " code_msg " << code_msg << endl;
        client_handle_activ(nClientDesc, code_msg);

        test_connection_th_off = true;
        th.join();

        close(nClientDesc);
    }

    close(nSocketDesc);
    cout << "#DEBUG-accept_connections_activ: Closed" << endl;
}
