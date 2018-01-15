#include "main.hpp"

void manage_activ()
{
    struct sockaddr_in serverAddr;
    int code_msg;
    int nFoo = 1;
    int posX;
    int posY;
    int activeUsers;
    ofstream fileIn;
    fstream fileOut;
    string line;
    int bytesSR;

    while(reconnect_ed)
    {
        socketDescA = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socketDescA < 0)
        {
            cout << "#ERROR-manage_activ: Failed create socket!!!" << endl;
            usleep(1000 * 4); // 4 seconds
            continue;
        }
        setsockopt(socketDescA, SOL_SOCKET, SO_REUSEADDR, (char *) &nFoo, sizeof(nFoo));


        serverAddr.sin_addr.s_addr = inet_addr(servIPaddr.c_str());
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(servPORT_A);

        if(connect(socketDescA, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        {
            cout <<"#ERROR-manage_activ: Cannot connect to server!!!" << endl;
            close(socketDescA);
            usleep(1000 * 4); // 4 seconds
            continue;
        }

        cout <<"#DEBUG-manage_activ: loop started" << endl;
        while(!end_program)
        {
            // **********CHECK ACTIVE OTHER CLIENTS**********
            usleep(1000 * 4); // 4 seconds
            code_msg = 333;
            bytesSR = send(socketDescA, &code_msg, sizeof(code_msg), 0);
            cout << "#DEBUG-manage_activ: send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDescA); end_program = true;  break;}

            bytesSR = recv(socketDescA, &activeUsers, sizeof(activeUsers),0);
            cout << "#DEBUG-manage_activ: recv bytes " << bytesSR << endl;
            while(1)
            {
                fileIn.open("temp/activusr.txt");
                if(fileIn.is_open()) break;
            }
            fileIn << activeUsers;
            fileIn.close();

            // **********SEND SELECTED TEXT BY SELF**********
            usleep(1000 * 4); // 4 seconds
            code_msg = 444;
            posX = posY = 0;
            while(1)
            {
                fileOut.open("temp/selecpos.txt");
                if(fileOut.is_open()) break;
            }
            getline(fileOut, line);
            posX = atoi(line.c_str());
            getline(fileOut, line);
            posY = atoi(line.c_str());
            fileOut.close();

            bytesSR = send(socketDescA, &code_msg, sizeof(code_msg), 0);
            cout << "#DEBUG-manage_activ: send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDescA); end_program = true;  break;}
            bytesSR = send(socketDescA, &posX, sizeof(posX), 0);
            cout << "#DEBUG-manage_activ: send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDescA); end_program = true;  break;}
            bytesSR = send(socketDescA, &posY, sizeof(posY), 0);
            cout << "#DEBUG-manage_activ: send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDescA); end_program = true;  break;}

            // **********DOWNLOAD SELECTED TEXT BY OTHERS**********
            usleep(1000 * 4); // 4 seconds
            code_msg = 555;
            activeUsers = 0;
            bytesSR = send(socketDescA, &code_msg, sizeof(code_msg), 0);
            cout << "#DEBUG-manage_activ: send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDescA); end_program = true;  break;}

            bytesSR = recv(socketDescA, &activeUsers, sizeof(activeUsers), 0);
            cout << "#DEBUG-manage_activ: recv bytes " << bytesSR << endl;
            while(1)
            {
                fileIn.open("temp/selecposother.txt");
                if(fileIn.is_open()) break;
            }
            if(activeUsers != 0)
            {
                fileIn << activeUsers << '\n';
                for(int i = 0; i < activeUsers; i++)
                {
                    bytesSR = recv(socketDescA, &posX, sizeof(posX), 0);
                    cout << "#DEBUG-manage_activ: recv bytes " << bytesSR << endl;
                    bytesSR = recv(socketDescA, &posY, sizeof(posY), 0);
                    cout << "#DEBUG-manage_activ: recv bytes " << bytesSR << endl;

                    fileIn << posX << '\n';
                    fileIn << posY << '\n';
                }
            }
            else
            {
              fileIn << 0 << '\n';
              fileIn << 0 << '\n';
              fileIn << 0 << '\n';
            }
            fileIn.close();
        }

        cout << "#DEBUG-manage_activ:CONNECTION FAILURE, WAIT TO RECONNECT" << endl;
        usleep(1000 * 6); // 6 seconds
    }
}
