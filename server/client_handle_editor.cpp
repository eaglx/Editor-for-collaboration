#include "main.hpp"

bool client_handle_editor(int nClientDesc_HE, int code_msg_HE)
{
    int bytesSR;
    char chr;
    int posX, posY;

    uint32_t network_byte_order_long;
    uint16_t network_byte_order_short;

    //cout << "#DEBUG-client_handle_editor: manage code_msg " << code_msg_HE << endl;
    //cout << "#DEBUG-client_handle_editor: manage descriptor " << nClientDesc_HE << endl;

    usleep(500);

    if(code_msg_HE == 111)
    {
        int temp;

        for(int i = 0; i < CLIENT_LIMIT; i++)
            if(CST[i].descriptor == nClientDesc_HE)
            {
                if(CST[i].allupdate == false)
                {
                    temp = 99;
                    i = 100;
                }
                else
                {
                    temp = 88;
                    i = 100;
                }
            }

        if(temp == 99)
        {
            network_byte_order_long = htonl(temp);
            bytesSR = send(nClientDesc_HE, &network_byte_order_long, sizeof(uint32_t), 0);
            //cout << "#DEBUG-client_handle_editor: temp send bytes " << bytesSR << endl;
            if(bytesSR < 0) return false;

            for(int i = 0; i < PAGE_X; i++)
                for(int j = 0; j < PAGE_Y; j++)
                {
                    network_byte_order_short = htons(bufforFE[i][j]);
                    bytesSR = send(nClientDesc_HE, &network_byte_order_short, sizeof(uint16_t), 0);
                    //cout << "#DEBUG-client_handle_editor: loop send bytes " << bytesSR << endl;
                    if(bytesSR < 0) return false;
                }

            for(int i = 0; i < CLIENT_LIMIT; i++)
                if(CST[i].descriptor == nClientDesc_HE)
                {
                    CST[i].allupdate = true;
                    i = 100;
                }
        }
        else
        {
            network_byte_order_long = htonl(temp);
            bytesSR = send(nClientDesc_HE, &network_byte_order_long, sizeof(uint32_t), 0);
            //cout << "#DEBUG-client_handle_editor: send bytes " << bytesSR << endl;
            if(bytesSR < 0) return false;
        }
    }
    else if(code_msg_HE == 222)
    {
        bytesSR = recv(nClientDesc_HE, &network_byte_order_short, sizeof(uint16_t), 0);
        chr = ntohs(network_byte_order_short);
        //cout << "#DEBUG-client_handle_editor: recv bytes " << bytesSR << endl;
        if(bytesSR < 0) return false;

        bytesSR = recv(nClientDesc_HE, &network_byte_order_long, sizeof(uint32_t), 0);
        posX = ntohl(network_byte_order_long);
        //cout << "#DEBUG-client_handle_editor: recv bytes " << bytesSR << endl;
        if(bytesSR < 0) return false;

        bytesSR = recv(nClientDesc_HE, &network_byte_order_long, sizeof(uint32_t), 0);
        posY = ntohl(network_byte_order_long);
        //cout << "#DEBUG-client_handle_editor: recv bytes " << bytesSR << endl;
        if(bytesSR < 0) return false;

        if(((posX >= 0) && (posX < PAGE_X)) && ((posY >= 0) && (posY < PAGE_Y)))
        {
            bufforFE[posX][posY] = chr;

            for(int i = 0; i < CLIENT_LIMIT; i++)
                if(CST[i].descriptor != nClientDesc_HE)
                    CST[i].allupdate = false;
        }
    }
    else
    {
        cout << "#DEBUG-client_handle_editor: DFQ!!!! from " << nClientDesc_HE << endl;
        //cout << "#DEBUG-client_handle_activ: Close client due security reason > " << nClientDesc_HE << endl;
        return false;
    }
    return true;
}
