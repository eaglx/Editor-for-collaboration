#ifndef MAIN_HPP
#define MAIN_HPP

#include <algorithm>
#include <arpa/inet.h>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/poll.h>
#include <thread>
#include <unistd.h>
#include <vector>

#define QUEUE_SIZE 20
#define PORT 6666

using namespace std;

struct MESSAGE_INFO
{
    int flag;
    char chr;
    int posX;
    int posY;
};

#endif
