#include "mainwindow.h"
#include <QApplication>
#include "connectdatamanage.h"

std::ofstream logFile;
volatile bool isEndProgram = false;
std::string dataFromQTextEdit;
std::string dataFromServer;
int socketDesc;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    int returnedValueEventLoop;

    struct sockaddr_in serverAddr;
    char buffer[50];
    int byteGet;

    std::ifstream configFile("config_file.conf");
    std::string ip_addr;
    int servPORT;
    std::string temp;

    logFile.open("log.txt");
    if (!logFile.is_open())
    {
        return -1;
    }

    if(configFile.is_open())
    {
      getline(configFile, ip_addr);
      getline(configFile, temp);
      servPORT = atoi(temp.c_str());
      configFile.close();
      logFile << "#DEBUG: configurations loaded\n";
    }
    else
    {
        logFile <<"#ERROR: no config found!\n";
        return -1;
    }

    socketDesc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketDesc < 0)
    {
        logFile << "#ERROR: Failed create socket!!!\n";
        logFile.close();
        return -2;
    }

    serverAddr.sin_addr.s_addr = inet_addr(ip_addr.c_str());
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(servPORT);

    if(connect(socketDesc, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
    {
        logFile << "#ERROR: Cannot connect to server!!!\n";
        logFile.close();
        return -3;
    }
    dataFromServer = "";
    for(int i = 0; i < 50; i++) { buffer[i] = '\0'; }
    while(true)
    {
        byteGet = recv(socketDesc, &buffer, sizeof(char) * 50, 0);
        logFile << "#INFO: recv bytes " << byteGet << "\n";
        if(byteGet < 0)
        {
            logFile << "#ERROR: recv\n";
            close(socketDesc);
            logFile.close();
            return -4;
        }
        else if(byteGet == 0) break;

        dataFromServer = dataFromServer + std::string(buffer);
        if(byteGet < int(sizeof(char) * 50)) break;
    }
    std::thread listenTH(listen_from_server, &w);
    w.show();
    returnedValueEventLoop = a.exec();
    logFile << "#INFO: Event loop return value " << returnedValueEventLoop << "\n";
    isEndProgram = true;
    close(socketDesc);
    logFile << "#INFO: Wait for threads\n";
    listenTH.join();
    logFile << "#INFO: The client successfully closed\n";
    logFile.close();
    return returnedValueEventLoop;
}
