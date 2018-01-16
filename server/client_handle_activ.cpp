#include "main.hpp"

void client_handle_activ(int nClientDesc_ACV, int code_msg_ACV)
{
    int bytesSR;

    cout << "#DEBUG-client_handle_activ: manage code_msg " << code_msg_ACV << endl;
    cout << "#DEBUG-client_handle_activ: manage descriptor " << nClientDesc_ACV << endl;

    if(code_msg_ACV == 333)
    {
        int temp = numberClientsDescriptors - 1;
        bytesSR = send(nClientDesc_ACV, &temp, sizeof(temp), 0);
        cout << "#DEBUG-client_handle_activ: send bytes " << bytesSR << endl;
    }
    else if(code_msg_ACV == 444)
    {
        for(unsigned int i = 0; i < clientsDescriptorsACA.size(); i++)
            if(clientsDescriptorsACA[i].desc == nClientDesc_ACV)
            {
                bytesSR = recv(nClientDesc_ACV, &clientsDescriptorsACA[i].selectStart, sizeof(clientsDescriptorsACA[i].selectStart), 0);
                cout << "#DEBUG-client_handle_activ: recv bytes " << bytesSR << endl;
                bytesSR = recv(nClientDesc_ACV, &clientsDescriptorsACA[i].selectEnd, sizeof(clientsDescriptorsACA[i].selectEnd), 0);
                cout << "#DEBUG-client_handle_activ: recv bytes " << bytesSR << endl;
                cout << "#DEBUG-client_handle_activ: client selected postitions " << clientsDescriptorsACA[i].selectStart << " " << clientsDescriptorsACA[i].selectEnd << endl;
                i = 100;
            }
    }
    else if(code_msg_ACV == 555)
    {
        int tempSCD;
        tempSCD = numberClientsDescriptors - 1;
        if(tempSCD != 0)
        {
            bytesSR = send(nClientDesc_ACV, &tempSCD, sizeof(tempSCD), 0);
            cout << "#DEBUG-client_handle_activ: lot clients, send bytes " << bytesSR << endl;

            for(unsigned int i = 0; i < clientsDescriptorsACA.size(); i++)
                if(clientsDescriptorsACA[i].desc != nClientDesc_ACV)
                {
                    bytesSR = send(nClientDesc_ACV, &clientsDescriptorsACA[i].selectStart, sizeof(clientsDescriptorsACA[i].selectStart), 0);
                    cout << "#DEBUG-client_handle_activ: send bytes " << bytesSR << endl;
                    bytesSR = send(nClientDesc_ACV, &clientsDescriptorsACA[i].selectEnd, sizeof(clientsDescriptorsACA[i].selectEnd), 0);
                    cout << "#DEBUG-client_handle_activ: send bytes " << bytesSR << endl;
                    cout << "#DEBUG-client_handle_activ: others selected postitions " << clientsDescriptorsACA[i].selectStart << " " << clientsDescriptorsACA[i].selectEnd << endl;
                }
        }
        else
        {
          tempSCD = 0;
          bytesSR = send(nClientDesc_ACV, &tempSCD, sizeof(tempSCD), 0);
          cout << "#DEBUG-client_handle_activ: only one client, send bytes " << bytesSR << endl;
        }
    }
    else
    {
        cout << "#DEBUG-client_handle_activ: DFQ!!!! from " << nClientDesc_ACV << endl;
    }
}
