#include "mainwindow.h"
#include <QApplication>
#include "connectdatamanage.h"

std::string strBuffer;
std::ofstream logFile;
volatile bool isEndProgram = false;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    int returnedValueEventLoop;

    int socketDesc;
    struct sockaddr_in serverAddr;
    char buffer[50];
    int byteGet;

    logFile.open("log.txt");
    if (!logFile.is_open())
    {
        return -1;
    }

    socketDesc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketDesc < 0)
    {
        logFile << "#ERROR: Failed create socket!!!\n";
        //logFile.close();
        //return -1;
    }

    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6666);

    if(connect(socketDesc, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
    {
        logFile << "#ERROR: Cannot connect to server!!!\n";
        //logFile.close();
        //return -2;
    }
    /*
    strBuffer = "";
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
                return -2;
            }
            else if(byteGet == 0) break;

            strBuffer = strBuffer + std::string(buffer);
            if(byteGet < int(sizeof(char) * 50)) break;
        }
    */

    std::thread listenTH(listen_from_server, socketDesc, &w);
    //std::thread sendTH(send_to_server, socketDesc);
    w.show();
    returnedValueEventLoop = a.exec();
    logFile << "#INFO: Event loop return value " << returnedValueEventLoop << "\n";
    isEndProgram = true;
    logFile << "#INFO: Wait for threads\n";
    listenTH.join();
    //sendTH.join();
    close(socketDesc);
    logFile << "#INFO: The client successfully closed\n";
    logFile.close();
    return returnedValueEventLoop;
}
