#include "main.hpp"

bool client_handle_editor(int nClientDesc, int code_msg)
{
    int bytesSR;
    char chr;
    int posX, posY;
    usleep(1000 * 1); //1 sec

    cout << "#DEBUG-manage_client: manage code_msg " << code_msg << endl;
    cout << "#DEBUG-manage_client: manage descriptor " << nClientDesc << endl;
    if(code_msg == 111)
    {
        int temp;

        for(int i = 0; i < CLIENT_LIMIT; i++)
            if(CST[i].descriptor == nClientDesc)
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
            bytesSR = send(nClientDesc, &temp, sizeof(temp), 0);
            cout << "#DEBUG-client_handle: temp send bytes " << bytesSR << endl;
            for(int i = 0; i < PAGE_X; i++)
                for(int j = 0; j < PAGE_Y; j++)
                {
                    bytesSR = send(nClientDesc, &bufforFE[i][j], sizeof(bufforFE[i][j]), 0);
                    cout << "#DEBUG-client_handle: loop send bytes " << bytesSR << endl;
                    if(bytesSR < 0)
                        return false;
                }

            for(int i = 0; i < CLIENT_LIMIT; i++)
                if(CST[i].descriptor == nClientDesc)
                {
                    CST[i].allupdate = true;
                    i = 100;
                }
        }
        else
        {
            bytesSR = send(nClientDesc, &temp, sizeof(temp), 0);
            //cout << "#DEBUG-client_handle: send bytes " << bytesSR << endl;
            if(bytesSR < 0) return false;
        }
    }
    else if(code_msg == 222)
    {
        bytesSR = recv(nClientDesc, &chr, sizeof(chr), 0);
        //cout << "#DEBUG-client_handle: recv bytes " << bytesSR << endl;
        if(bytesSR < 0) return false;
        bytesSR = recv(nClientDesc, &posX, sizeof(posX), 0);
        //cout << "#DEBUG-client_handle: recv bytes " << bytesSR << endl;
        if(bytesSR < 0) return false;
        bytesSR = recv(nClientDesc, &posY, sizeof(posY), 0);
        //cout << "#DEBUG-client_handle: recv bytes " << bytesSR << endl;
        if(bytesSR < 0) return false;

        if(!(posX >= 0) && (posX < PAGE_X)) return false;
        if(!(posY >= 0) && (posY < PAGE_Y)) return false;

        bufforFE[posX][posY] = chr;

        for(int i = 0; i < CLIENT_LIMIT; i++)
            if(CST[i].descriptor != nClientDesc)
                CST[i].allupdate = false;
    }
    else if(code_msg == 333)
    {
        int temp = numberClientsDescriptors - 1;
        bytesSR = send(nClientDesc, &temp, sizeof(temp), 0);
        //cout << "#DEBUG-client_handle: send bytes " << bytesSR << endl;
        if(bytesSR < 0) return false;
    }
    else if(code_msg == 444)
    {
        for(int i = 0; i < CLIENT_LIMIT; i++)
            if(CST[i].descriptor == nClientDesc)
            {
                bytesSR = recv(nClientDesc, &CST[i].selectStart, sizeof(CST[i].selectStart), 0);
                //cout << "#DEBUG-client_handle: recv bytes " << bytesSR << endl;
                bytesSR = recv(nClientDesc, &CST[i].selectEnd, sizeof(CST[i].selectEnd), 0);
                //cout << "#DEBUG-client_handle: recv bytes " << bytesSR << endl;
                i = 100;
            }
    }
    else if(code_msg == 555)
    {
        int tempSCD = 0;
        tempSCD = numberClientsDescriptors - 1;
        if(tempSCD != 0)
        {
            bytesSR = send(nClientDesc, &tempSCD, sizeof(tempSCD), 0);
            cout << "#DEBUG-client_handle: tempSCD send bytes " << bytesSR << endl;
            if(bytesSR < 0) return false;
            for(int i = 0; i < CLIENT_LIMIT; i++)
                if((CST[i].descriptor != -1) && (CST[i].descriptor != nClientDesc))
                {
                    bytesSR = send(nClientDesc, &CST[i].selectStart, sizeof(CST[i].selectStart), 0);
                    cout << "#DEBUG-client_handle: send bytes " << bytesSR << endl;
                    if(bytesSR < 0) return false;
                    bytesSR = send(nClientDesc, &CST[i].selectEnd, sizeof(CST[i].selectEnd), 0);
                    cout << "#DEBUG-client_handle: send bytes " << bytesSR << endl;
                    if(bytesSR < 0) return false;
                }
        }
        else
        {
          tempSCD = 0;
          bytesSR = send(nClientDesc, &tempSCD, sizeof(tempSCD), 0);
          //cout << "#DEBUG-client_handle: send bytes " << bytesSR << endl;
          if(bytesSR < 0) return false;
        }
    }
    else if(code_msg == 666)
    {
        cout << "#DEBUG: Close client due he finished work > " << nClientDesc << endl;
        return false;
    }
    else
    {
        cout << "#DEBUG: DFQ!!!!" << endl;
        cout << "#DEBUG: Close client due security reason > " << nClientDesc << endl;
        return false;
    }
    return true;
}
