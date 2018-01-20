#include "main.hpp"

pollfd *waitfor = NULL;

void pollfd_array_resize()
{
    cout << "#DEBUG***********: pollfd_array_resize" << endl;
    if(waitfor != NULL)
    {
        delete waitfor;
        waitfor = NULL;
    }
    cout << "#DEBUG-pollfd_array_resize: numberClientsDescriptors = " << numberClientsDescriptors << endl;
    if(numberClientsDescriptors != 0)
    {
        waitfor = new pollfd[numberClientsDescriptors];
        for(size_t i = 0; i < clientsDescriptors.size(); i++)
        {
            waitfor[i].fd = clientsDescriptors[i];
            waitfor[i].events = POLLIN;
            cout << "#DEBUG-pollfd_array_resize: Active descriptions " << clientsDescriptors[i] << endl;
        }
    }
}

void control_client()
{
    cout << "#DEBUG: control_client lounched" << endl;

    int readypoll;
    int codeMsg;
    int bytesSR;

    while(!end_program)
    {
        unique_lock<std::mutex> lk(cv_m);
        cv.wait(lk, []{return READY_THREAD_GLOBAL_SYNC;});

        if(numberClientsDescriptors == 0)
        {
            READY_THREAD_GLOBAL_SYNC = false;
            continue;
        }

        if(numberClientsDescriptorsChang == true)
        {
            numberClientsDescriptorsChang = false;
            pollfd_array_resize();
        }

        if(numberClientsDescriptors != 0)
        {
            readypoll = poll(waitfor, numberClientsDescriptors, 5000);
            if(readypoll == -1)
            {
                cout << "#DEBUG-control_client: POLL ERROR" << endl;
                continue;
            }
            else if(readypoll == 0)
            {
                cout <<"#DEBUG-control_client: POLL TIMEOUT" << endl;
                for(unsigned int i = 0; i < clientsDescriptors.size(); i++)
                {
                    cout <<"#DEBUG-control_client: Test connection to descriptor " << waitfor[i].fd << endl;
                    codeMsg = send(waitfor[i].fd, &codeMsg, sizeof(codeMsg), MSG_NOSIGNAL);
                    if ((codeMsg == -1) || (CST[i].timeoutcount == 3))
                    {
                        --numberClientsDescriptors;
                        clientsDescriptors.erase(clientsDescriptors.begin() + i);
                        numberClientsDescriptorsChang = true;

                        cout << "#DEBUG-control_client: Delete due timeout client id " << waitfor[i].fd << endl;
                        for(int i = 0; i < CLIENT_LIMIT; i++)
                        {
                            if(CST[i].descriptor == waitfor[i].fd)
                            {
                                close(waitfor[i].fd);
                                CST[i].descriptor = -1;
                                CST[i].clientSPECIAL_ID = -1;
                                CST[i].timeoutcount = 0;
                                i = CLIENT_LIMIT;
                            }
                        }
                        continue;
                    }
                    else
                    {
                        cout << "#DEBUG-control_client: Test connection to port OK" << endl;
                        ++CST[i].timeoutcount;
                    }
                }
            }
            else
            {
                if(waitfor != NULL)
                {
                    for(int i = 0; i < numberClientsDescriptors; i++)
                        if(waitfor[i].revents & POLLIN)
                        {
                            bytesSR = recv(waitfor[i].fd, &codeMsg, sizeof(codeMsg), 0);
                            //cout << "#DEBUG: recv bytes before manage_client " << bytesSR << endl;
                            if(bytesSR < 0)
                            {
                                --numberClientsDescriptors;
                                clientsDescriptors.erase(clientsDescriptors.begin() + i);

                                cout << "#DEBUG-control_client: control_client Delete client id - " << waitfor[i].fd << endl;
                                for(int i = 0; i < CLIENT_LIMIT; i++)
                                    if(CST[i].descriptor == waitfor[i].fd)
                                    {
                                        close(waitfor[i].fd);
                                        CST[i].descriptor = -1;
                                        CST[i].clientSPECIAL_ID = -1;
                                        CST[i].timeoutcount = 0;
                                    }
                            }
                            else
                            {
                                if(!client_handle_editor(waitfor[i].fd, codeMsg))
                                {
                                    cout << "#DEBUG-control_client: control_client Delete client id - " << waitfor[i].fd << endl;
                                    close(waitfor[i].fd);
                                    CST[i].descriptor = -1;
                                    CST[i].clientSPECIAL_ID = -1;
                                    CST[i].timeoutcount = 0;
                                }
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

    cout << "#DEBUG-control_client: control_client closed" << endl;
}
