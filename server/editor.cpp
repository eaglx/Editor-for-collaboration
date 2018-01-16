#include "main.hpp"

struct Plik *plik = NULL;

void feditor()
{
    struct msgbuf
    {
        long type;
        char ch;
        int posX;
        int posY;
    };

    plik = new Plik;

    struct msgbuf fifo;
    int idF;
    int typeGet = 10;

    idF = msgget(123456, 0644|IPC_CREAT);
    if(idF == -1)
    {
        cout << "#ERROR: Cannot create IPC!!!" << endl;
        delete plik;
        return;
    }

    for(int i = 0; i < PAGE_X; i++)
        for(int j = 0; j < PAGE_Y; j++)
            plik->buffor[i][j] = '\0';

    cout << "#DEBUG: feditor ready." << endl;
    while(!end_program)
    {
        while(msgrcv(idF, &fifo, sizeof(fifo) - sizeof(long), typeGet, 0) > 0)
        {
            plik->buffor[fifo.posX][fifo.posY] = fifo.ch;
        }
    }

    if(plik != NULL) 
        delete plik;
    cout << "#DEBUG: feditor is shutdown" << endl;
}
