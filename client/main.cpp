#include "main.hpp"

int socketDesc;
string servIPaddr;
int servPORT;
bool end_program = false;
bool reconnect = true;

struct tm *foo;
struct stat attrib;

void signal_callback_handler(int signum)
{
  cout << "#DEBUG-client: Signum = " << signum <<endl;
  end_program = true;
  reconnect = false;
  int code_msg = 666;
  send(socketDesc, &code_msg, sizeof(code_msg), 0);
  close(socketDesc);
  cout << "#DEBUG-client: Start shutdown client" << endl;
}

void signal_callback_handler_PIPE(int signum)
{
    cout << "#ERROR: caught signal SIGPIPE " << signum << "!!!!!!" << endl;
    end_program = true;
    close(socketDesc);
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
    if(!load_config(servIPaddr, servPORT)) exit(-1);

    signal(SIGINT, signal_callback_handler);
    signal(SIGPIPE, signal_callback_handler_PIPE);

    thread cfeth(check_files_existance);
    thread meth(manage_editor);

    cout << "#DEBUG-client: Wait for threads" << endl;
    cfeth.join();
    meth.join();
    cout << "#DEBUG-client: Client closed" << endl;

    return 0;
}
