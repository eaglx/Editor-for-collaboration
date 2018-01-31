#include "mainwindow.h"
#include <QApplication>
#include "connectdatamanage.h"

std::string strBuffer;
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

    socketDesc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketDesc < 0)
    {
        // Save to file log
        // << "#ERROR-client: Failed create socket!!!";
        return -1;
    }

    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(6666);

    if(connect(socketDesc, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
    {
        // Save to file log
        // << "#ERROR-client: Cannot connect to server!!!";
        return -2;
    }

    strBuffer = "";
    for(int i = 0; i < 50; i++) { buffer[i] = '\0'; }
        while(true)
        {
            byteGet = recv(socketDesc, &buffer, sizeof(char) * 50, 0);
            //cout << "recv bytes " << byteGet << endl;
            if(byteGet < 0)
            {
                //cout << "#ERROR-client: recv" << endl;
                close(socketDesc);
                return -2;
            }
            else if(byteGet == 0) break;

            strBuffer = strBuffer + std::string(buffer);
            if(byteGet < int(sizeof(char) * 50)) break;
        }

    //std::thread listenTH(listen_from_server, socketDesc);
    //std::thread sendTH(send_to_server, socketDesc);
    w.show();
    returnedValueEventLoop = a.exec(); // Event loop is launched.
    isEndProgram = true;
    //listenTH.join();
    //sendTH.join();
    close(socketDesc);
    return returnedValueEventLoop;
}
