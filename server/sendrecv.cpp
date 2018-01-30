#include "include/main.hpp"

int send_all(int socket, void *buffer, size_t length)
{
    char *ptr = (char*) buffer;
    int i;

    while (length > 0)
    {
        i = send(socket, ptr, length, 0);
        if (i < 1) return SEND_ERROR;
        ptr += i;
        length -= i;
    }

    return SEND_ALL_DATA;
}

int recv_all(int socket, void *buffer)
{
    char *ptr = (char*) buffer;
    int i;
    int length = 0;
    int iteration = 0;
    bool loopFinish = false;

    while (!loopFinish)
    {
        i = recv(socket, ptr, length, 0);
        if (i == -1) return RECIVE_ERROR;
        else if(i == 0)
        {
            if(iteration == 0) return RECIVE_ZERO;
            else loopFinish = true;
        }
        ptr += i;
        length += i;
        ++iteration;
    }

    return length;
}
