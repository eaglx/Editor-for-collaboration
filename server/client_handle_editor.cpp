#include "main.hpp"

void client_handle_editor(int nClientDesc_HE, int code_msg_HE)
{
    int bytesSR;
    char chr;
    int posX, posY;

    //cout << "#DEBUG-client_handle_editor: manage code_msg " << code_msg_HE << endl;
    //cout << "#DEBUG-client_handle_editor: manage descriptor " << nClientDesc_HE << endl;
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
            bytesSR = send(nClientDesc_HE, &temp, sizeof(temp), 0);
            //cout << "#DEBUG-client_handle_editor: temp send bytes " << bytesSR << endl;
            for(int i = 0; i < PAGE_X; i++)
                for(int j = 0; j < PAGE_Y; j++)
                {
                    bytesSR = send(nClientDesc_HE, &bufforFE[i][j], sizeof(bufforFE[i][j]), 0);
                    //cout << "#DEBUG-client_handle_editor: loop send bytes " << bytesSR << endl;
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
            bytesSR = send(nClientDesc_HE, &temp, sizeof(temp), 0);
            //cout << "#DEBUG-client_handle_editor: send bytes " << bytesSR << endl;
        }
    }
    else if(code_msg_HE == 222)
    {
        bytesSR = recv(nClientDesc_HE, &chr, sizeof(chr), 0);
        //cout << "#DEBUG-client_handle_editor: recv bytes " << bytesSR << endl;
        bytesSR = recv(nClientDesc_HE, &posX, sizeof(posX), 0);
        //cout << "#DEBUG-client_handle_editor: recv bytes " << bytesSR << endl;
        bytesSR = recv(nClientDesc_HE, &posY, sizeof(posY), 0);
        //cout << "#DEBUG-client_handle_editor: recv bytes " << bytesSR << endl;

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
    }
}
