#ifndef MAIN_HPP
#define MAIN_HPP

#include <arpa/inet.h>
#include <iostream>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <cstdlib>
#include <string>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/msg.h>

#include <signal.h>

#include <chrono>
#include <condition_variable>
#include <thread>

#include <vector>
#include <cstring>

#include <sys/poll.h>

#define QUEUE_SIZE 10
#define PORT 6666

#define PAGE_X 26
#define PAGE_Y 45
#define CLIENT_LIMIT 7

using namespace std;

extern bool end_program;
extern condition_variable cv;
extern mutex cv_m;
extern bool ready;
extern int id;
extern struct Plik *plik;
extern int numberClientsDescriptors;
extern struct ClientSelectText CST[CLIENT_LIMIT];

/* FUNCTIONS */
void manage_client(int nClientDesc);
void feditor();

/* STRUCT */
struct Plik
{
    char buffor[PAGE_X][PAGE_Y];
};

struct ClientSelectText
{
    int descriptor;
    int selectStart;
    int selectEnd;
    bool allupdate;
};

#endif
