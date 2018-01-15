#include "main.hpp"

int socketDescE;
int socketDescA;
string servIPaddr;
int servPORT_A;
int servPORT_E;
bool end_program_e = false;
bool end_program_a = false;
bool reconnect_ed = true;
bool reconnect_acv = true;

struct tm *foo;
struct stat attrib;

void signal_callback_handler(int signum)
{
  cout << "#DEBUG-client: Signum = " << signum <<endl;
  end_program_a = end_program_e = true;
  reconnect_acv = reconnect_ed = false;
  close(socketDescE);
  close(socketDescA);
  cout << "#DEBUG-client: Start shutdown client" << endl;
}

void signal_callback_handler_PIPE(int signum)
{
    cout << "#ERROR: caught signal SIGPIPE " << signum << "!!!!!!" << endl;
}

void check_files_existance()
{
    bool end_loop = false;
    cout <<"#DEBUG-client-th-check_existance: thread-check_existance running" << endl;
    struct stat dirStat;
    while(!end_loop)
    {
        if(stat("temp", &dirStat) != -1)
        {
            if(S_ISDIR(dirStat.st_mode) == 0)
            {
                end_loop = true;
                raise(SIGINT);
                cout <<"#DEBUG-client-th-check_existance: raised SIGINT" << endl;
            }
        }
        else
        {
            end_loop = true;
            raise(SIGINT);
            cout <<"#DEBUG-client-th-check_existance: raised SIGINT" << endl;
        }
    }
    cout <<"#DEBUG-client-th-check_existance: thread-check_existance stop" << endl;
    exit(0);
}

bool load_config()
{
    ifstream configFile("config_file.conf");
    string temp;
    if(configFile.is_open())
    {
      getline(configFile, servIPaddr);
      getline(configFile, temp);
      servPORT_E = atoi(temp.c_str());
      getline(configFile, temp);
      servPORT_A = atoi(temp.c_str());
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
    if(!load_config()) exit(-1);

    signal(SIGINT, signal_callback_handler);
    signal(SIGPIPE, signal_callback_handler_PIPE);

    thread cfeth(check_files_existance);
    thread meth(manage_editor);
    thread math(manage_activ);

    cout << "#DEBUG-client: Wait for threads" << endl;
    cfeth.join();
    meth.join();
    math.join();
    cout << "#DEBUG-client: Client closed" << endl;

    return 0;
}
