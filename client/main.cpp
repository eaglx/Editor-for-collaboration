#include "main.hpp"

int socketDesc;
bool end_program = false;

struct tm *foo;
struct stat attrib;

void signal_callback_handler(int signum)
{
  cout << "#DEBUG: Signum = " << signum <<endl;
  end_program = true;
  close(socketDesc);
  cout << "#DEBUG: Start shutdown client" << endl;
}

void update_file_info(int &lastModifyHour, int &lastModifyMin, int &lastModifySec)
{
    stat("temp/out.txt", &attrib);
    foo = gmtime(&(attrib.st_mtime));
    lastModifyHour = foo->tm_hour;
    lastModifyMin = foo->tm_min;
    lastModifySec = foo->tm_sec;
}

int main()
{
    signal(SIGINT, signal_callback_handler);
    struct sockaddr_in serverAddr;
    int code_msg;

    char buffor[PAGE_X][PAGE_Y];
    for(int i = 0; i < PAGE_X; i++)
        for(int j = 0; j < PAGE_Y; j++)
            buffor[i][j] = '\0';

    unsigned int px, py;
    char chr;
    int posX;
    int posY;
    int activeUsers;

    ofstream fileIn;
    fstream fileOut;
    string line;

    int lastModifyHour = 0;
    int lastModifyMin = 0;
    int lastModifySec = 0;
    bool isModify = false;

    socketDesc = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(socketDesc < 0)
    {
        cout << "#ERROR: Failed create socket!!!" << endl;
        return -1;
    }

    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    if(connect(socketDesc, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
    {
        cout <<"#ERROR: Cannot connect to server!!!" << endl;
        return -2;
    }


    code_msg = 111;
    write(socketDesc, &code_msg, sizeof(code_msg));
    write(socketDesc, &code_msg, sizeof(code_msg));
    read(socketDesc, &code_msg, sizeof(code_msg));
    for(int i = 0; i < PAGE_X; i++)
        for(int j = 0; j < PAGE_Y; j++)
            read(socketDesc, &buffor[i][j], sizeof(buffor[i][j]));
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

    update_file_info(lastModifyHour, lastModifyMin, lastModifySec);

    while(!end_program)
    {
        // **********SEND CHANGES IN EDITED FILE**********
        usleep(1000 * 1); // 1 seconds
        stat("temp/out.txt", &attrib);
        foo = gmtime(&(attrib.st_mtime));
        if(lastModifyHour == foo->tm_hour)
        {
            if(lastModifyMin == foo->tm_min)
            {
                if(lastModifySec == foo->tm_sec)
                {
                    isModify = false;
                    //cout << "#DEBUG: out.txt not modify" << endl;
                }
                else isModify = true;
            }
            else isModify = true;
        }
        else isModify = true;

        if(isModify == true)
        {
            update_file_info(lastModifyHour, lastModifyMin, lastModifySec);

            cout << "#DEBUG: out.txt is modify" << endl;

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
                  buffor[py][px+1] = '\n';          // NEED ???
                ++py;
            }
            fileOut.close();

            for(; py < PAGE_X; py++)
                for(int i = 0; i < PAGE_Y; i++)
                    buffor[py][i] = '\0';

            for(int i = 0; i < PAGE_X; i++)
                for(int j = 0; j < PAGE_Y; j++)
                {
                    chr = buffor[i][j];
                    posX = i;
                    posY = j;

                    code_msg = 222;
                    //cout << "#DEBUG: code_msg: " << code_msg << endl;
                    write(socketDesc, &code_msg, sizeof(code_msg));
                    write(socketDesc, &code_msg, sizeof(code_msg));
                    write(socketDesc, &chr, sizeof(chr));
                    write(socketDesc, &posX, sizeof(posX));
                    write(socketDesc, &posY, sizeof(posY));
                }
        }

        // **********DOWNLOAD 'EDITED' FILE**********
        usleep(1000 * 1); // 1 seconds
        code_msg = 111;
        //cout << "#DEBUG: code_msg: " << code_msg << endl;
        write(socketDesc, &code_msg, sizeof(code_msg));
        write(socketDesc, &code_msg, sizeof(code_msg));

        read(socketDesc, &code_msg, sizeof(code_msg));
        if(code_msg == 99)
        {
            cout << "#DEBUG: Update from server" << endl;
            isModify = true;
            for(int i = 0; i < PAGE_X; i++)
                for(int j = 0; j < PAGE_Y; j++)
                    read(socketDesc, &buffor[i][j], sizeof(buffor[i][j]));
        }
        else
        {
            //cout << "#DEBUG: All updated" << endl;
            isModify = false;
        }

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

            update_file_info(lastModifyHour, lastModifyMin, lastModifySec);
        }

        // **********CHECK ACTIVE OTHER CLIENTS**********
        usleep(1000 * 1); // 1 seconds
        code_msg = 333;
        //cout << "#DEBUG: code_msg: " << code_msg << endl;
        write(socketDesc, &code_msg, sizeof(code_msg));
        write(socketDesc, &code_msg, sizeof(code_msg));
        read(socketDesc, &activeUsers, sizeof(activeUsers));
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
        //cout << "#DEBUG: code_msg: " << code_msg << endl;
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
        if(posX != posY)
        {
            //cout << "#DEBUG: User select pos: " << posX << ":" << posY << endl;
            write(socketDesc, &code_msg, sizeof(code_msg));
            write(socketDesc, &code_msg, sizeof(code_msg));
            write(socketDesc, &posX, sizeof(posX));
            write(socketDesc, &posY, sizeof(posY));
        }

        // **********DOWNLOAD SELECTED TEXT BY OTHERS**********
        usleep(1000 * 1); // 1 seconds
        code_msg = 555;
        //cout << "#DEBUG: code_msg: " << code_msg << endl;
        activeUsers = 0;
        write(socketDesc, &code_msg, sizeof(code_msg));
        write(socketDesc, &code_msg, sizeof(code_msg));
        read(socketDesc, &activeUsers, sizeof(activeUsers));
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
              read(socketDesc, &posX, sizeof(posX));
              read(socketDesc, &posY, sizeof(posY));
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
    close(socketDesc);
    return 0;
}
