#include "main.hpp"

void clrBuff(char (&buffor)[PAGE_X][PAGE_Y])
{
    for(int i = 0; i < PAGE_X; i++)
        for(int j = 0; j < PAGE_Y; j++) buffor[i][j] = '\0';
}

void update_file_info(int &lastModifySec)
{
    stat("temp/out.txt", &attrib);
    foo = gmtime(&(attrib.st_mtime));
    lastModifySec = foo->tm_sec;
}


void manage_editor()
{
    struct sockaddr_in serverAddr;
    int code_msg;
    int nFoo = 1;
    char buffor[PAGE_X][PAGE_Y];
    unsigned int px, py;
    char chr;
    int posX;
    int posY;
    int activeUsers;
    ofstream fileIn;
    fstream fileOut;
    string line;
    int lastModifySec;
    bool isModify;
    int bytesSR;

    while(reconnect)
    {
        end_program = false;
        lastModifySec = 0;
        isModify = false;
        clrBuff(buffor);

        socketDesc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socketDesc < 0)
        {
            cout << "#ERROR-client: Failed create socket!!!" << endl;
            while(1)
            {
                fileIn.open("temp/out.txt");
                if(fileIn.is_open()) break;
            }
            fileIn << "#ERROR-client: Failed create socket!!!";
            fileIn.close();
            continue;
        }
        setsockopt(socketDesc, SOL_SOCKET, SO_REUSEADDR, (char *) &nFoo, sizeof(nFoo));


        serverAddr.sin_addr.s_addr = inet_addr(servIPaddr.c_str());
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(servPORT);

        if(connect(socketDesc, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        {
            cout <<"#ERROR-client: Cannot connect to server!!!" << endl;
            while(1)
            {
                fileIn.open("temp/out.txt");
                if(fileIn.is_open()) break;
            }
            fileIn << "#ERROR-client: Cannot connect to server!!!";
            fileIn.close();
            continue;
        }

        code_msg = 111;
        bytesSR = send(socketDesc, &code_msg, sizeof(code_msg), 0);
        cout << "#DEBUG: send bytes before big loop" << bytesSR << endl;
        if(bytesSR < 0) { close(socketDesc); continue; }

        bytesSR = recv(socketDesc, &code_msg, sizeof(code_msg), 0);
        cout << "#DEBUG: recv bytes before big loop" << bytesSR << endl;
        for(int i = 0; i < PAGE_X; i++)
            for(int j = 0; j < PAGE_Y; j++)
            {
                bytesSR = recv(socketDesc, &buffor[i][j], sizeof(buffor[i][j]), 0);
                cout << "#DEBUG: recv bytes in small loop" << bytesSR << endl;
            }
        while(1)
        {
            fileIn.open("temp/out.txt");
            if(fileIn.is_open()) break;
        }
        for(py = 0; py < PAGE_X; py++)
            for(px = 0; px < PAGE_Y; px++)
                if(buffor[py][px] != '\0')
                    fileIn << buffor[py][px];
        fileIn.close();

        update_file_info(lastModifySec);

        int loopCount = 0;
        cout <<"#DEBUG-client: loop started" << endl;
        while(!end_program)
        {
            // **********SEND CHANGES IN EDITED FILE**********
            usleep(1000 * 1); // 1 seconds
            stat("temp/out.txt", &attrib);
            foo = gmtime(&(attrib.st_mtime));
            if(lastModifySec == foo->tm_sec) isModify = false;
            else { lastModifySec = foo->tm_sec; isModify = true; }

            if(isModify == true)
            {
                cout << "#DEBUG-client: out.txt is modify" << endl;

                while(1)
                {
                    fileOut.open("temp/out.txt");
                    if(fileOut.is_open()) break;
                }
                py = 0;
                while(getline(fileOut, line))
                {
                    for(px = 0; px < line.length(); px++)
                        buffor[py][px] = line[px];
                    if(buffor[py][px] != '\n')
                        buffor[py][px+1] = '\n';
                    ++py;
                }
                fileOut.close();

                for(; py < PAGE_X; py++)
                    for(int i = 0; i < PAGE_Y; i++)
                        buffor[py][i] = '\0';

                /*for(int i = 0; i < PAGE_X; i++)
                    for(int j = 1; j < PAGE_Y; j++)
                        if(buffor[i][j-1] == buffor[i][j])
                            if(buffor[i][j] == '\n')
                                buffor[i][j] = '\0';*/

                for(int i = 0; i < PAGE_X; i++)
                    for(int j = 0; j < PAGE_Y; j++)
                    {
                        chr = buffor[i][j];
                        posX = i;
                        posY = j;

                        code_msg = 222;
                        bytesSR = send(socketDesc, &code_msg, sizeof(code_msg),0);
                        cout << "#DEBUG: send bytes " << bytesSR << endl;
                        if(bytesSR < 0) { close(socketDesc); end_program = true;  break;}
                        bytesSR = send(socketDesc, &chr, sizeof(chr),0);
                        cout << "#DEBUG: send bytes " << bytesSR << endl;
                        if(bytesSR < 0) { close(socketDesc); end_program = true;  break;}
                        bytesSR = send(socketDesc, &posX, sizeof(posX),0);
                        cout << "#DEBUG: send bytes " << bytesSR << endl;
                        if(bytesSR < 0) { close(socketDesc); end_program = true;  break;}
                        bytesSR = send(socketDesc, &posY, sizeof(posY),0);
                        cout << "#DEBUG: send bytes " << bytesSR << endl;
                        if(bytesSR < 0) { close(socketDesc); end_program = true;  break;}
                    }
            }

            // **********DOWNLOAD 'EDITED' FILE**********
            usleep(1000 * 1); // 1 seconds
            code_msg = 111;
            bytesSR = send(socketDesc, &code_msg, sizeof(code_msg), 0);
            cout << "#DEBUG: send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDesc); end_program = true;  break;}

            bytesSR = recv(socketDesc, &code_msg, sizeof(code_msg),0);
            cout << "#DEBUG: recv bytes " << bytesSR << endl;
            if(code_msg == 99)
            {
                cout << "#DEBUG-client: Update from server" << endl;
                isModify = true;
                for(int i = 0; i < PAGE_X; i++)
                    for(int j = 0; j < PAGE_Y; j++)
                    {
                        bytesSR = recv(socketDesc, &buffor[i][j], sizeof(buffor[i][j]),0);
                        cout << "#DEBUG: recv bytes " << bytesSR << endl;
                    }
            }
            else isModify = false;

            if(isModify == true)
            {
                while(1)
                {
                    fileIn.open("temp/out.txt");
                    if(fileIn.is_open()) break;
                }
                for(py = 0; py < PAGE_X; py++)
                    for(px = 0; px < PAGE_Y; px++)
                        if(buffor[py][px] != '\0')
                            fileIn << buffor[py][px];
                fileIn.close();

                update_file_info(lastModifySec);
            }

            if(loopCount > 3) { loopCount = 0; clrBuff(buffor); continue; }
            // **********CHECK ACTIVE OTHER CLIENTS**********
            usleep(1000 * 1); // 1 seconds
            code_msg = 333;
            bytesSR = send(socketDesc, &code_msg, sizeof(code_msg), 0);
            cout << "#DEBUG: send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDesc); end_program = true;  break;}

            bytesSR = recv(socketDesc, &activeUsers, sizeof(activeUsers),0);
            cout << "#DEBUG: recv bytes " << bytesSR << endl;
            while(1)
            {
                fileIn.open("temp/activusr.txt");
                if(fileIn.is_open()) break;
            }
            fileIn << activeUsers;
            fileIn.close();

            // **********SEND SELECTED TEXT BY SELF**********
            usleep(1000 * 1); // 1 seconds
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

            bytesSR = send(socketDesc, &code_msg, sizeof(code_msg), 0);
            cout << "#DEBUG: send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDesc); end_program = true;  break;}
            bytesSR = send(socketDesc, &posX, sizeof(posX), 0);
            cout << "#DEBUG: send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDesc); end_program = true;  break;}
            bytesSR = send(socketDesc, &posY, sizeof(posY), 0);
            cout << "#DEBUG: send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDesc); end_program = true;  break;}

            // **********DOWNLOAD SELECTED TEXT BY OTHERS**********
            usleep(1000 * 1); // 1 seconds
            code_msg = 555;
            activeUsers = 0;
            bytesSR = send(socketDesc, &code_msg, sizeof(code_msg), 0);
            cout << "#DEBUG: send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDesc); end_program = true;  break;}

            bytesSR = recv(socketDesc, &activeUsers, sizeof(activeUsers), 0);
            cout << "#DEBUG: recv bytes " << bytesSR << endl;
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
                    bytesSR = recv(socketDesc, &posX, sizeof(posX), 0);
                    cout << "#DEBUG: recv bytes " << bytesSR << endl;
                    bytesSR = recv(socketDesc, &posY, sizeof(posY), 0);
                    cout << "#DEBUG: recv bytes " << bytesSR << endl;

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
            ++loopCount;
        }

        while(1)
        {
            fileIn.open("temp/out.txt");
            if(fileIn.is_open()) break;
        }
        fileIn << "CONNECTION FAILURE, WAIT TO RECONNECT";
        fileIn.close();
        usleep(1000 * 6); // 6 seconds
    }
}
