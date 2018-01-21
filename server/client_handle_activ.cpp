#include "main.hpp"

bool client_handle_activ(int nClientDesc_ACV, int code_msg_ACV)
{
    int bytesSR;
    uint32_t network_byte_order_long;

    //cout << "#DEBUG-client_handle_activ: manage code_msg " << code_msg_ACV << endl;
    //cout << "#DEBUG-client_handle_activ: manage descriptor " << nClientDesc_ACV << endl;

    usleep(500);

    if(code_msg_ACV == 333)
    {
        int temp = numberClientsDescriptorsACA - 1;
        network_byte_order_long = htonl(temp);
        bytesSR = send(nClientDesc_ACV, &network_byte_order_long, sizeof(uint32_t), 0);
        //cout << "#DEBUG-client_handle_activ: send bytes " << bytesSR << endl;
        if(bytesSR < 0) return false;
    }
    else if(code_msg_ACV == 444)
    {
        for(unsigned int i = 0; i < clientsDescriptorsACA.size(); i++)
            if(clientsDescriptorsACA[i].desc == nClientDesc_ACV)
            {
                bytesSR = recv(nClientDesc_ACV, &network_byte_order_long, sizeof(uint32_t), 0);
                //cout << "#DEBUG-client_handle_activ: recv bytes " << bytesSR << endl;
                clientsDescriptorsACA[i].selectStart = ntohl(network_byte_order_long);
                if(bytesSR < 0) return false;

                bytesSR = recv(nClientDesc_ACV, &network_byte_order_long, sizeof(uint32_t), 0);
                //cout << "#DEBUG-client_handle_activ: recv bytes " << bytesSR << endl;
                clientsDescriptorsACA[i].selectEnd = ntohl(network_byte_order_long);
                if(bytesSR < 0) return false;
                //cout << "#DEBUG-client_handle_activ: client selected postitions " << clientsDescriptorsACA[i].selectStart << " " << clientsDescriptorsACA[i].selectEnd << endl;
                i = 100;
            }
    }
    else if(code_msg_ACV == 555)
    {
        int tempSCD;
        tempSCD = numberClientsDescriptorsACA - 1;
        if(tempSCD != 0)
        {
            network_byte_order_long = htonl(tempSCD);
            bytesSR = send(nClientDesc_ACV, &network_byte_order_long, sizeof(uint32_t), 0);
            if(bytesSR < 0) return false;
            //cout << "#DEBUG-client_handle_activ: lot clients, send bytes " << bytesSR << endl;

            for(unsigned int i = 0; i < clientsDescriptorsACA.size(); i++)
                if(clientsDescriptorsACA[i].desc != nClientDesc_ACV)
                {
                    network_byte_order_long = htonl(clientsDescriptorsACA[i].selectStart);
                    bytesSR = send(nClientDesc_ACV, &network_byte_order_long, sizeof(uint32_t), 0);
                    //cout << "#DEBUG-client_handle_activ: send bytes " << bytesSR << endl;
                    if(bytesSR < 0) return false;

                    network_byte_order_long = htonl(clientsDescriptorsACA[i].selectEnd);
                    bytesSR = send(nClientDesc_ACV, &network_byte_order_long, sizeof(uint32_t), 0);
                    //cout << "#DEBUG-client_handle_activ: send bytes " << bytesSR << endl;
                    if(bytesSR < 0) return false;
                    //cout << "#DEBUG-client_handle_activ: others selected postitions " << clientsDescriptorsACA[i].selectStart << " " << clientsDescriptorsACA[i].selectEnd << endl;
                }
        }
        else
        {
          tempSCD = 0;
          network_byte_order_long = htonl(tempSCD);
          bytesSR = send(nClientDesc_ACV, &network_byte_order_long, sizeof(uint32_t), 0);
          //cout << "#DEBUG-client_handle_activ: only one client, send bytes " << bytesSR << endl;
          if(bytesSR < 0) return false;
        }
    }
    else
    {
        cout << "#DEBUG-client_handle_activ: DFQ!!!! from " << nClientDesc_ACV << endl;
        cout << "#DEBUG-client_handle_activ: Close client due security reason > " << nClientDesc_ACV << endl;
        return false;
    }
    return true;
}
