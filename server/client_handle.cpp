#include "main.hpp"

bool manage_client(int nClientDesc, int code_msg)
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
            write(nClientDesc, &temp, sizeof(temp));
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
        fifo.ch = chr;
        read(nClientDesc, &posX, sizeof(posX));
        fifo.posX = posX;
        read(nClientDesc, &posY, sizeof(posY));
        fifo.posY = posY;
        fifo.type = 10;

        if(!(posX >= 0) && (posX < PAGE_X)) return true;
        if(!(posY >= 0) && (posY < PAGE_Y)) return true;

        status = msgsnd(id, &fifo, sizeof(fifo) - sizeof(long), 0);
        if(status != 0)
        {
            cout << "#DEBUG: MSGSND ERROR!!!" << endl;
            return true;
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
