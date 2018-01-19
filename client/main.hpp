#ifndef _MAIN_HPP
#define _MAIN_HPP

#include <arpa/inet.h>
#include <iostream>
#include <fstream>
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
#include <time.h>
#include <signal.h>
#include <thread>

#define PAGE_X 26
#define PAGE_Y 45

using namespace std;

extern int clientSPECIAL_ID;
extern int socketDescE;
extern int socketDescA;
extern string servIPaddr;
extern int servPORT_E;
extern int servPORT_A;
extern bool end_program_e;
extern bool end_program_a;
extern bool reconnect_ed;
extern bool reconnect_acv;
extern struct tm *foo;
extern struct stat attrib;

void manage_editor();
void manage_activ();

#endif
