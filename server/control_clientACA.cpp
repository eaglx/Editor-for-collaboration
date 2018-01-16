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
            cout << "#DEBUG-pollfd_array_resize_ACA: Active descriptions " << clientsDescriptors[i] << endl;
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
        unique_lock<std::mutex> lk2(cv_mACA);
        cvACA.wait(lk2, []{return READY_THREAD_GLOBAL_SYNC_ACA;});

        if(numberClientsDescriptorsACA == 0)
        {
            lk2.unlock();
            cvACA.notify_all();
            continue;
        }

        if(numberClientsDescriptorsChangACA == true)
        {
            numberClientsDescriptorsChangACA = false;
            pollfd_array_resize_ACA();
        }

        if(numberClientsDescriptorsACA != 0)
        {
            readypoll = poll(waitforACA, numberClientsDescriptorsACA, 1000);
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
                for(unsigned int i = 0; i < clientsDescriptorsACA.size(); i++)
                {
                    cout <<"#DEBUG-control_clientACA: test client with special id" << clientsDescriptorsACA[i].id  << endl;
                    bytesSR  = send(clientsDescriptorsACA[i].desc, &codeMsg, sizeof(codeMsg), MSG_NOSIGNAL);
                    if(bytesSR == -1)
                    {
                        cout <<"#DEBUG-control_clientACA: close special id" << clientsDescriptorsACA[i].id  << endl;
                        close(clientsDescriptorsACA[i].desc);
                        clientsDescriptorsACA.erase(clientsDescriptorsACA.begin() + i);
                        numberClientsDescriptorsChangACA = true;
                        i = numberClientsDescriptorsACA;
                        --numberClientsDescriptorsACA;
                    }
                }
                lk2.unlock();
                cvACA.notify_all();
                continue;
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
                                //cout << "#DEBUG-control_clientACA: recv bytes " << bytesSR << " code_msg " << codeMsg << endl;
                                if(!client_handle_activ(waitforACA[i].fd, codeMsg))
                                {
                                    cout << "#DEBUG-control_client: control_client Delete client special id - " << clientsDescriptorsACA[i].id << endl;
                                    close(waitforACA[i].fd);
                                    clientsDescriptorsACA.erase(clientsDescriptorsACA.begin() + i);
                                    numberClientsDescriptorsChangACA = true;
                                    --numberClientsDescriptorsACA;
                                }
                            }
                            else
                            {
                                cout << "#DEBUG-control_client: control_client Delete client special id - " << clientsDescriptorsACA[i].id << endl;
                                close(waitforACA[i].fd);
                                clientsDescriptorsACA.erase(clientsDescriptorsACA.begin() + i);
                                numberClientsDescriptorsChangACA = true;
                                --numberClientsDescriptorsACA;
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
