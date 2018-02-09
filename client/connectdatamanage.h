#ifndef CONNECTDATAMANAGE
#define CONNECTDATAMANAGE

#include "mainwindow.h"

#include <arpa/inet.h>
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
#include <sys/stat.h>
#include <thread>
#include <fstream>

#define FLAG_INSERT_BEFORE 111
#define FLAG_REPLACE 222
#define FLAG_APPEND 333
#define FLAG_RM 444
#define FLAG_DEL_ALL 555

#define INTERNALFLAG_LOCK_EDITABLE 111
#define INTERNALFLAG_UNLOCK_EDITABLE 222
#define INTERNALFLAG_UPDATE_DATA 333

#define SEND_ALL_DATA 0
#define SEND_ERROR -1
#define RECIVE_ZERO 0
#define RECIVE_ERROR -1
#define PACKETSIZE sizeof(MESSAGE_INFO)

struct MESSAGE_INFO
{
    int flag;
    int posX;
    char chr;
};

void listen_from_server(int, MainWindow *);
void send_to_server(int, MainWindow *);
void serialize_msg(MESSAGE_INFO *, char *);
void deserialize_msg(char *, MESSAGE_INFO *);

#endif // CONNECTDATAMANAGE
