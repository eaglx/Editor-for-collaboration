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
extern bool READY_THREAD_GLOBAL_SYNC;
extern int nSocketDesc;
extern char bufforFE[PAGE_X][PAGE_Y];
extern vector < int > clientsDescriptors;
extern int numberClientsDescriptors;
extern struct ClientSelectText CST[CLIENT_LIMIT];
extern bool numberClientsDescriptorsChang;

/* FUNCTIONS */
bool client_handle_editor(int nClientDesc, int code_msg);
void control_client();
void accept_connections();

/* STRUCT */
struct ClientSelectText
{
    int descriptor;
    int selectStart;
    int selectEnd;
    int timeoutcount;
    bool allupdate;
};

#endif
