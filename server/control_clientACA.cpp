#include "main.hpp"


void pollfd_array_resize_ACA()
{
    cout << "#DEBUG***********: pollfd_array_resize_ACA" << endl;

    if(waitforACA != NULL)
    {
        delete waitforACA;
        waitforACA = NULL;
    }

    if(numberClientsDescriptorsACA != 0)
    {
        waitforACA = new pollfd[numberClientsDescriptorsACA];
        cout << "#DEBUG-pollfd_array_resize_ACA: Active clients number " << numberClientsDescriptorsACA << endl;
        for(int i = 0; i < numberClientsDescriptorsACA; i++)
        {
            waitforACA[i].fd = clientsDescriptorsACA[i].desc;
            waitforACA[i].events = POLLIN;
            cout << "#DEBUG-pollfd_array_resize_ACA: Active descriptions " << clientsDescriptorsACA[i].desc << endl;
        }
    }
}

void control_clientACA()
{
    int readypoll;
    int codeMsg;
    int bytesSR;
    int timeout = 0;
    uint32_t network_byte_order_long;

    cout << "#DEBUG-accept_connections_activ: control_clientACA run" << endl;
    while(!manage_thread_ACA)
    {
        unique_lock<std::mutex> lk2(cv_mACA);
        cvACA.wait(lk2, []{return READY_THREAD_GLOBAL_SYNC_ACA;});

        if(numberClientsDescriptorsACA == 0)
        {
            cout << "#DEBUG-accept_connections_activ: no clients $$$$$$$" << endl;
            delete waitforACA;
            waitforACA = NULL;
            clientsDescriptorsACA.clear();
            READY_THREAD_GLOBAL_SYNC_ACA = false;
            continue;
        }

        if(numberClientsDescriptorsChangACA == true)
        {
            numberClientsDescriptorsChangACA = false;
            pollfd_array_resize_ACA();
        }

        if(numberClientsDescriptorsACA != 0)
        {
            readypoll = poll(waitforACA, numberClientsDescriptorsACA, 2000);
            if(readypoll == -1)
            {
                cout << "#DEBUG-control_clientACA: POLL ERROR" << endl;
                continue;
            }
            else if(readypoll == 0)
            {
                cout <<"#DEBUG-control_clientACA: POLL TIMEOUT" << endl;
                for(int i = 0; i < numberClientsDescriptorsACA; i++)
                {
                    cout << "#DEBUG-control_clientACA: test client desc " << waitforACA[i].fd  << endl;
                    network_byte_order_long = htonl(codeMsg);
                    bytesSR  = send(waitforACA[i].fd, &network_byte_order_long, sizeof(uint32_t), MSG_NOSIGNAL);
                    if(bytesSR == -1)
                    {
                        cout << "#DEBUG-control_clientACA: close client desc " << waitforACA[i].fd  << endl;
                        close(waitforACA[i].fd);
                        for(unsigned int j = 0; j < clientsDescriptorsACA.size(); j++)
                            if(clientsDescriptorsACA[j].desc == waitforACA[i].fd)
                            {
                                clientsDescriptorsACA.erase(clientsDescriptorsACA.begin() + j);
                            }
                        numberClientsDescriptorsChangACA = true;
                        --numberClientsDescriptorsACA;
                        timeout = 0;
                    }
                    else ++timeout;
                }

                if(timeout >= 3)
                {
                    cout <<"#DEBUG-control_clientACA: TIMEOUT, DISCONECT ALL" << endl;
                    for(int i = 0; i < numberClientsDescriptorsACA; i++) close(waitforACA[i].fd);
                    clientsDescriptorsACA.clear();
                    numberClientsDescriptorsACA = 0;
                    delete(waitforACA);
                    waitforACA = NULL;
                    timeout = 0;
                    usleep(8000000); // 8 seconds
                    cout << "#DEBUG-control_clientACA: recontinue after sec sleep ^$$^" << endl;
                }

                continue;
            }
            else
            {
                if(waitforACA != NULL)
                {
                    for(int i = 0; i < numberClientsDescriptorsACA; i++)
                        if(waitforACA[i].revents & POLLIN)
                        {
                            bytesSR = recv(waitforACA[i].fd, &network_byte_order_long, sizeof(uint32_t), 0);
                            codeMsg = ntohl(network_byte_order_long);
                            if(bytesSR > 0)
                            {
                                //cout << "#DEBUG-control_clientACA: recv bytes " << bytesSR << " code_msg " << codeMsg << endl;
                                if(!client_handle_activ(waitforACA[i].fd, codeMsg))
                                {
                                    /*cout <<"#DEBUG-control_clientACA:  control_clientACA Delete client descc" << waitforACA[i].fd << endl;
                                    close(waitforACA[i].fd);
                                    for(unsigned int j = 0; j < clientsDescriptorsACA.size(); j++)
                                        if(clientsDescriptorsACA[j].desc == waitforACA[i].fd)
                                        {
                                            cout <<"#DEBUG-control_clientACA:  control_clientACA Delete client desc " << clientsDescriptorsACA[j].desc  << endl;
                                            clientsDescriptorsACA.erase(clientsDescriptorsACA.begin() + j);
                                        }
                                    --numberClientsDescriptorsACA;
                                    i = 1000;
                                    numberClientsDescriptorsChangACA = true;*/
                                    ;
                                }
                            }
                            else
                            {
                                cout <<"#DEBUG-control_clientACA:  control_clientACA Delete client desc " << waitforACA[i].fd  << endl;
                                close(waitforACA[i].fd);
                                for(unsigned int j = 0; j < clientsDescriptorsACA.size(); j++)
                                    if(clientsDescriptorsACA[j].desc == waitforACA[i].fd)
                                        {
                                            cout <<"#DEBUG-control_clientACA:  control_clientACA Delete client desc " << clientsDescriptorsACA[j].desc  << endl;
                                            clientsDescriptorsACA.erase(clientsDescriptorsACA.begin() + j);
                                        }
                                --numberClientsDescriptorsACA;
                                i = 1000;
                                numberClientsDescriptorsChangACA = true;
                            }
                        }
                }
            }
        }
    }
    cout << "#DEBUG-accept_connections_activ: control_clientACA stop" << endl;
}
