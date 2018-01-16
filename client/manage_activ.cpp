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

    usleep(200000); // 0.2 seconds
    while(reconnect_ed)
    {
        socketDescA = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socketDescA < 0)
        {
            cout << "#ERROR-manage_activ: Failed create socket!!!" << endl;
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
            continue;
        }

        send(socketDescA, &clientSPECIAL_ID, sizeof(clientSPECIAL_ID), 0);

        cout <<"#DEBUG-manage_activ: loop started" << endl;
        while(!end_program_a)
        {
            usleep(200000); // 0.2 seconds
            // **********CHECK ACTIVE OTHER CLIENTS**********
            code_msg = 333;
            bytesSR = send(socketDescA, &code_msg, sizeof(code_msg), 0);
            //cout << "#DEBUG-manage_activ: 333 send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDescA); end_program_a = true;  break;}

            bytesSR = recv(socketDescA, &activeUsers, sizeof(activeUsers),0);
            //cout << "#DEBUG-manage_activ: 333 recv bytes " << bytesSR << endl;
            while(1)
            {
                fileIn.open("temp/activusr.txt");
                if(fileIn.is_open()) break;
            }
            fileIn << activeUsers;
            fileIn.close();

            // **********SEND SELECTED TEXT BY SELF**********
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

            if(posX == posY) posX = posY = 0;

            //cout << "#DEBUG-manage_activ: selected positions " << posX << " " << posY << endl;

            bytesSR = send(socketDescA, &code_msg, sizeof(code_msg), 0);
            //cout << "#DEBUG-manage_activ: 444 send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDescA); end_program_a = true;  break;}
            bytesSR = send(socketDescA, &posX, sizeof(posX), 0);
            //cout << "#DEBUG-manage_activ: 444 send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDescA); end_program_a = true;  break;}
            bytesSR = send(socketDescA, &posY, sizeof(posY), 0);
            //cout << "#DEBUG-manage_activ: 444 send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDescA); end_program_a = true;  break;}

            // **********DOWNLOAD SELECTED TEXT BY OTHERS**********
            code_msg = 555;
            activeUsers = 0;
            bytesSR = send(socketDescA, &code_msg, sizeof(code_msg), 0);
            //cout << "#DEBUG-manage_activ: 555 send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDescA); end_program_a = true;  break;}

            bytesSR = recv(socketDescA, &activeUsers, sizeof(activeUsers), 0);
            //cout << "#DEBUG-manage_activ: 555 recv bytes " << bytesSR << endl;
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
                    //cout << "#DEBUG-manage_activ: 555 recv bytes " << bytesSR << endl;
                    bytesSR = recv(socketDescA, &posY, sizeof(posY), 0);
                    //cout << "#DEBUG-manage_activ: 555 recv bytes " << bytesSR << endl;

                    //cout << "#DEBUG-manage_activ: selected positions by others " << posX << " " << posY << endl;

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
        close(socketDescA);
        cout << "#DEBUG-manage_activ:CONNECTION FAILURE, WAIT TO RECONNECT" << endl;
        usleep(4000000); // 4 seconds
    }
}
