#include "main.hpp"

int socketDesc;
bool end_program = false;

struct tm *foo;
struct stat attrib;

void signal_callback_handler(int signum)
{
  cout << "#DEBUG-client: Signum = " << signum <<endl;
  end_program = true;
  int code_msg = 666;
  write(socketDesc, &code_msg, sizeof(code_msg));
  close(socketDesc);
  cout << "#DEBUG-client: Start shutdown client" << endl;
}

void signal_callback_handler_PIPE(int signum)
{
    cout << "#ERROR: caught signal SIGPIPE " << signum << "!!!!!!" << endl;
    //close(socketDesc);
}

void update_file_info(int &lastModifyMin, int &lastModifySec)
{
    stat("temp/out.txt", &attrib);
    foo = gmtime(&(attrib.st_mtime));
    lastModifyMin = foo->tm_min;
    lastModifySec = foo->tm_sec;
}

void check_existance()
{
    cout <<"#DEBUG-client: thread-check_existance running" << endl;
    struct stat dirStat;
    while(!end_program)
    {
        if(stat("temp", &dirStat) != -1)
        {
            if(S_ISDIR(dirStat.st_mode) == 0)
                end_program = true;
        }
        else
            end_program = true;
    }
    cout <<"#DEBUG-client: thread-check_existance stop" << endl;
    raise(SIGINT);
    exit(0);
}

void clrBuff(char (&buffor)[PAGE_X][PAGE_Y])
{
    for(int i = 0; i < PAGE_X; i++)
        for(int j = 0; j < PAGE_Y; j++) buffor[i][j] = '\0';
}

bool load_config(string &ip_addr, int &port)
{
    ifstream configFile("config_file.conf");
    string temp;
    if(configFile.is_open())
    {
      getline(configFile, ip_addr);
      getline(configFile, temp);
      port = atoi(temp.c_str());
      configFile.close();
      cout <<"#DEBUG-client: configurations loaded" << endl;
      return true;
    }
    else
    {
        cout <<"#DEBUG-client: no config!!!" << endl;
        return false;
    }
}

int main()
{
    string servIPaddr;
    int servPORT;
    if(!load_config(servIPaddr, servPORT)) exit(-1);

    signal(SIGINT, signal_callback_handler);
    signal(SIGPIPE, signal_callback_handler_PIPE);

    struct sockaddr_in serverAddr;
    int code_msg;

    char buffor[PAGE_X][PAGE_Y];
    clrBuff(buffor);

    unsigned int px, py;
    char chr;
    int posX;
    int posY;
    int activeUsers;

    ofstream fileIn;
    fstream fileOut;
    string line;

    int lastModifyMin = 0;
    int lastModifySec = 0;
    bool isModify = false;

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
        exit(-2);
    }
    int nFoo = 1;
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
        exit(-3);
    }

    code_msg = 111;
    if(write(socketDesc, &code_msg, sizeof(code_msg)) < 0) { close(socketDesc); return -1; }
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

    update_file_info(lastModifyMin, lastModifySec);

    thread th_1(check_existance);
    int loopCount = 0;
    cout <<"#DEBUG-client: loop started" << endl;
    while(!end_program)
    {
        // **********SEND CHANGES IN EDITED FILE**********
        usleep(1000 * 1); // 1 seconds
        stat("temp/out.txt", &attrib);
        foo = gmtime(&(attrib.st_mtime));
        if(lastModifyMin == foo->tm_min)
        {
            if(lastModifySec == foo->tm_sec)isModify = false;
            else isModify = true;
        }
        else isModify = true;

        if(isModify == true)
        {
            update_file_info(lastModifyMin, lastModifySec);

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
                    if(write(socketDesc, &code_msg, sizeof(code_msg)) < 0) { close(socketDesc); return -1; }
                    if(write(socketDesc, &chr, sizeof(chr)) < 0) { close(socketDesc); return -1; }
                    if(write(socketDesc, &posX, sizeof(posX)) < 0) { close(socketDesc); return -1; }
                    if(write(socketDesc, &posY, sizeof(posY)) < 0) { close(socketDesc); return -1; }
                }
        }

        // **********DOWNLOAD 'EDITED' FILE**********
        usleep(1000 * 1); // 1 seconds
        code_msg = 111;
        if(write(socketDesc, &code_msg, sizeof(code_msg)) < 0) { close(socketDesc); return -1; }

        read(socketDesc, &code_msg, sizeof(code_msg));
        if(code_msg == 99)
        {
            cout << "#DEBUG-client: Update from server" << endl;
            isModify = true;
            for(int i = 0; i < PAGE_X; i++)
                for(int j = 0; j < PAGE_Y; j++)
                    read(socketDesc, &buffor[i][j], sizeof(buffor[i][j]));
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

            update_file_info(lastModifyMin, lastModifySec);
        }

        if(loopCount > 3) { loopCount = 0; clrBuff(buffor); continue; }
        // **********CHECK ACTIVE OTHER CLIENTS**********
        usleep(1000 * 1); // 1 seconds
        code_msg = 333;
        if(write(socketDesc, &code_msg, sizeof(code_msg)) < 0) { close(socketDesc); return -1; }
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
        if(write(socketDesc, &code_msg, sizeof(code_msg)) < 0) { close(socketDesc); return -1; }
        if(write(socketDesc, &posX, sizeof(posX)) < 0) { close(socketDesc); return -1; }
        if(write(socketDesc, &posY, sizeof(posY)) < 0) { close(socketDesc); return -1; }

        // **********DOWNLOAD SELECTED TEXT BY OTHERS**********
        usleep(1000 * 1); // 1 seconds
        code_msg = 555;
        activeUsers = 0;
        if(write(socketDesc, &code_msg, sizeof(code_msg)) < 0) { close(socketDesc); return -1; }
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
        ++loopCount;
    }
    close(socketDesc);
    cout << "#DEBUG-client: Wait for thread" << endl;
    th_1.join();
    cout << "#DEBUG-client: Client closed" << endl;
    return 0;
}
