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
    ofstream fileIn;
    fstream fileOut;
    string line;
    int lastModifySec;
    bool isModify;
    int bytesSR;

    while(reconnect_ed)
    {
        end_program_e = false;
        lastModifySec = 0;
        isModify = false;
        clrBuff(buffor);

        socketDescE = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
        if(socketDescE < 0)
        {
            cout << "#ERROR-manage_editor: Failed create socket!!!" << endl;
            while(1)
            {
                fileIn.open("temp/out.txt");
                if(fileIn.is_open()) break;
            }
            fileIn << "#ERROR-manage_editor: Failed create socket!!!";
            fileIn.close();
            continue;
        }
        setsockopt(socketDescE, SOL_SOCKET, SO_REUSEADDR, (char *) &nFoo, sizeof(nFoo));


        serverAddr.sin_addr.s_addr = inet_addr(servIPaddr.c_str());
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(servPORT_E);

        if(connect(socketDescE, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
        {
            cout <<"#ERROR-manage_editor: Cannot connect to server!!!" << endl;
            while(1)
            {
                fileIn.open("temp/out.txt");
                if(fileIn.is_open()) break;
            }
            fileIn << "#ERROR-manage_editor: Cannot connect to server!!!";
            fileIn.close();
            continue;
        }

        send(socketDescE, &clientSPECIAL_ID, sizeof(clientSPECIAL_ID), 0);

        code_msg = 111;
        bytesSR = send(socketDescE, &code_msg, sizeof(code_msg), 0);
        //cout << "#DEBUG-manage_editor: send bytes before big loop" << bytesSR << endl;
        if(bytesSR < 0) { close(socketDescE); continue; }

        bytesSR = recv(socketDescE, &code_msg, sizeof(code_msg), 0);
        //cout << "#DEBUG-manage_editor: recv bytes before big loop" << bytesSR << endl;
        for(int i = 0; i < PAGE_X; i++)
            for(int j = 0; j < PAGE_Y; j++)
            {
                bytesSR = recv(socketDescE, &buffor[i][j], sizeof(buffor[i][j]), 0);
                //cout << "#DEBUG-manage_editor: recv bytes in small loop " << bytesSR << endl;
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
        cout <<"#DEBUG-manage_editor: loop started" << endl;
        while(!end_program_e)
        {
            // **********SEND CHANGES IN EDITED FILE**********
            usleep(500000); // 0.5 seconds
            stat("temp/out.txt", &attrib);
            foo = gmtime(&(attrib.st_mtime));
            if(lastModifySec == foo->tm_sec) isModify = false;
            else { lastModifySec = foo->tm_sec; isModify = true; }

            if(isModify == true)
            {
                cout << "#DEBUG-manage_editor: out.txt is modify" << endl;

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
                        bytesSR = send(socketDescE, &code_msg, sizeof(code_msg),0);
                        //cout << "#DEBUG-manage_editor: send bytes " << bytesSR << endl;
                        if(bytesSR < 0) { close(socketDescE); end_program_e = true;  break;}
                        bytesSR = send(socketDescE, &chr, sizeof(chr),0);
                        //cout << "#DEBUG-manage_editor: send bytes " << bytesSR << endl;
                        if(bytesSR < 0) { close(socketDescE); end_program_e = true;  break;}
                        bytesSR = send(socketDescE, &posX, sizeof(posX),0);
                        //cout << "#DEBUG-manage_editor: send bytes " << bytesSR << endl;
                        if(bytesSR < 0) { close(socketDescE); end_program_e = true;  break;}
                        bytesSR = send(socketDescE, &posY, sizeof(posY),0);
                        //cout << "#DEBUG-manage_editor: send bytes " << bytesSR << endl;
                        if(bytesSR < 0) { close(socketDescE); end_program_e = true;  break;}
                    }
            }

            // **********DOWNLOAD 'EDITED' FILE**********
            usleep(500000); // 0.5 seconds
            code_msg = 111;
            bytesSR = send(socketDescE, &code_msg, sizeof(code_msg), 0);
            //cout << "#DEBUG-manage_editor: send bytes " << bytesSR << endl;
            if(bytesSR < 0) { close(socketDescE); end_program_e = true;  break;}

            bytesSR = recv(socketDescE, &code_msg, sizeof(code_msg),0);
            //cout << "#DEBUG-manage_editor: recv bytes " << bytesSR << endl;
            if(code_msg == 99)
            {
                //cout << "#DEBUG-manage_editor: Update from server" << endl;
                isModify = true;
                for(int i = 0; i < PAGE_X; i++)
                    for(int j = 0; j < PAGE_Y; j++)
                    {
                        bytesSR = recv(socketDescE, &buffor[i][j], sizeof(buffor[i][j]),0);
                        //cout << "#DEBUG-manage_editor: recv bytes " << bytesSR << endl;
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

            ++loopCount;
            if(loopCount > 5) { loopCount = 0; clrBuff(buffor); }
        }

        while(1)
        {
            fileIn.open("temp/out.txt");
            if(fileIn.is_open()) break;
        }
        fileIn << "CONNECTION FAILURE, WAIT TO RECONNECT";
        fileIn.close();
        cout << "#DEBUG-manage_editor: CONNECTION FAILURE, WAIT TO RECONNECT" << endl;
        usleep(8000000); // 8 seconds
    }
}
