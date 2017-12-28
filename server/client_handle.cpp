#include "main.hpp"

void manage_client(int nClientDesc)
{
    struct msgbuf
    {
        long type;
        char ch;
        int posX;
        int posY;
    };

    struct msgbuf fifo;
    int status;

    char chr;
    int posX;
    int posY;

    int code_msg = 0;

    //cout << "#DEBUG: manage_client start, client id = " << nClientDesc << endl;
    if(read(nClientDesc, &code_msg, sizeof(code_msg)) < 0) return;
    //cout << "#DEBUG: manage_client get code - " << code_msg << endl;

    if(code_msg == 111)
    {
        int temp;

        for(int i = 0; i < CLIENT_LIMIT; i++)
            if(CST[i].descriptor == nClientDesc)
            {
                if(CST[i].allupdate == false)
                {
                    //cout << "#DEBUG:"<< nClientDesc << " allupdate status " << CST[i].allupdate  << endl;
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
            write(nClientDesc, &temp, sizeof(temp));
            //cout << "#DEBUG: client " << nClientDesc << " send update file" << endl;
            for(int i = 0; i < PAGE_X; i++)
                for(int j = 0; j < PAGE_Y; j++)
                    write(nClientDesc, &plik->buffor[i][j], sizeof(plik->buffor[i][j]));

            for(int i = 0; i < CLIENT_LIMIT; i++)
                if(CST[i].descriptor == nClientDesc)
                {
                    CST[i].allupdate = true;
                    i = 100;
                }
        }
        else
            write(nClientDesc, &temp, sizeof(temp));
    }
    else if(code_msg == 222)
    {
        read(nClientDesc, &chr, sizeof(chr));
        //cout << "#DEBUG: client_handle chr - " << chr << endl;
        fifo.ch = chr;
        read(nClientDesc, &posX, sizeof(posX));
        //cout << "#DEBUG: client_handle posX - " << posX << endl;
        fifo.posX = posX;
        read(nClientDesc, &posY, sizeof(posY));
        //cout << "#DEBUG: client_handle posY - " << posY << endl;
        fifo.posY = posY;
        fifo.type = 10;

        if(!(posX >= 0) && (posX < PAGE_X)) return;
        if(!(posY >= 0) && (posY < PAGE_Y)) return;

        status = msgsnd(id, &fifo, sizeof(fifo) - sizeof(long), 0);
        if(status != 0)
        {
            cout << "#DEBUG: MSGSND ERROR!!!" << endl;
            return;
        }

        for(int i = 0; i < CLIENT_LIMIT; i++)
            if(CST[i].descriptor != nClientDesc)
                CST[i].allupdate = false;
    }
    else if(code_msg == 333)
    {
        int temp = numberClientsDescriptors - 1;
        write(nClientDesc, &temp, sizeof(temp));
    }
    else if(code_msg == 444)
    {
        for(int i = 0; i < CLIENT_LIMIT; i++)
            if(CST[i].descriptor == nClientDesc)
            {
                read(nClientDesc, &CST[i].selectStart, sizeof(CST[i].selectStart));
                read(nClientDesc, &CST[i].selectEnd, sizeof(CST[i].selectEnd));
                i = 100;
            }
    }
    else if(code_msg == 555)
    {
        int tempSCD = 0;
        tempSCD = numberClientsDescriptors - 1;
        if((tempSCD != 0) && (tempSCD < CLIENT_LIMIT))
        {
            write(nClientDesc, &tempSCD, sizeof(tempSCD));
            for(int i = 0; i < CLIENT_LIMIT; i++)
                if((CST[i].descriptor != -1) && (CST[i].descriptor != nClientDesc))
                {
                    write(nClientDesc, &CST[i].selectStart, sizeof(CST[i].selectStart));
                    write(nClientDesc, &CST[i].selectEnd, sizeof(CST[i].selectEnd));
                }
        }
        else
        {
          tempSCD = 0;
          write(nClientDesc, &tempSCD, sizeof(tempSCD));
        }
    }
    else cout << "#DEBUG: DFQ!!!!" << endl;
}
