#include "main.hpp"

void pollfd_array_resize_ACA()
{
    if(waitforACA != NULL)
    {
        delete waitforACA;
        waitforACA = NULL;
    }

    if(numberClientsDescriptorsACA != 0)
    {
        waitforACA = new pollfd[numberClientsDescriptorsACA];
        for(size_t i = 0; i < clientsDescriptorsACA.size(); i++)
        {
            waitforACA[i].fd = clientsDescriptorsACA[i].desc;
            waitforACA[i].events = POLLIN;
        }
    }
}

void control_clientACA()
{
    int readypoll;
    int codeMsg;
    int bytesSR;

    cout << "#DEBUG-accept_connections_activ: control_clientACA run" << endl;
    while(!manage_thread_ACA)
    {
        usleep(500000); // 0.5 seconds
        if(numberClientsDescriptorsACA == 0) continue;

        unique_lock<std::mutex> lk2(cv_mACA);
        cvACA.wait(lk2, []{return READY_THREAD_GLOBAL_SYNC_ACA;});

        if(numberClientsDescriptorsChangACA == true)
        {
            numberClientsDescriptorsChangACA = false;
            pollfd_array_resize_ACA();
        }

        if(numberClientsDescriptorsACA != 0)
        {
            readypoll = poll(waitforACA, numberClientsDescriptors, 5000);
            if(readypoll == -1)
            {
                cout << "#DEBUG-control_clientACA: POLL ERROR" << endl;
                lk2.unlock();
                cvACA.notify_all();
                continue;
            }
            else if(readypoll == 0)
            {
                cout <<"#DEBUG-control_clientACA: POLL TIMEOUT" << endl;
            }
            else
            {
                if(waitforACA != NULL)
                {
                    for(int i = 0; i < numberClientsDescriptorsACA; i++)
                        if(waitforACA[i].revents & POLLIN)
                        {
                            bytesSR = recv(waitforACA[i].fd, &codeMsg, sizeof(codeMsg), 0);
                            if(bytesSR > 0)
                            {
                                cout << "#DEBUG-control_clientACA: recv bytes " << bytesSR << " code_msg " << codeMsg << endl;
                                client_handle_activ(waitforACA[i].fd, codeMsg);
                            }
                        }
                }
            }
        }

        lk2.unlock();
        cvACA.notify_all();
    }
    cout << "#DEBUG-accept_connections_activ: control_clientACA stop" << endl;
}

void test_connectionACA()
{
    char c;
    ssize_t x;
    cout << "#DEBUG-accept_connections_activ: test_connection_th run" << endl;
    while(!manage_thread_ACA)
    {
        usleep(1000000); // 1 seconds
        if(numberClientsDescriptorsACA == 0) continue;

        unique_lock<std::mutex> lk2(cv_mACA);
        cvACA.wait(lk2, []{return READY_THREAD_GLOBAL_SYNC_ACA;});

        for(int i = 0; i < numberClientsDescriptorsACA; i++)
        {
            x = recv(clientsDescriptorsACA[i].desc, &c, 1, MSG_PEEK);
            if (x > 0)
            {
                /* ...have data, leave it in socket buffer */
                cout << "#DEBUG-test_connection_th: Client exist" << endl;
            }
            else if (x == 0)
            {
                /* ...handle FIN from client */
                cout << "#DEBUG-test_connection_th: Close client - FIN" << endl;
                close(clientsDescriptorsACA[i].desc);
                clientsDescriptorsACA.erase(clientsDescriptorsACA.begin() + i);
                i = numberClientsDescriptorsACA;
                --numberClientsDescriptorsACA;
                numberClientsDescriptorsChangACA = true;
            }
            else
            {
                 /* ...handle errors */
                 cout << "#DEBUG-test_connection_th: Close client - error" << endl;
                close(clientsDescriptorsACA[i].desc);
                clientsDescriptorsACA.erase(clientsDescriptorsACA.begin() + i);
                i = numberClientsDescriptorsACA;
                --numberClientsDescriptorsACA;
                numberClientsDescriptorsChangACA = true;
            }
        }

        lk2.unlock();
        cvACA.notify_all();
    }
    cout << "#DEBUG-accept_connections_activ: test_connection_th stop" << endl;
}
