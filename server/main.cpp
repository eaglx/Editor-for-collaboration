#include "main.hpp"

char bufforFE[PAGE_X][PAGE_Y];

bool end_program = false;
int nSocketDesc;

condition_variable cv;
mutex cv_m;
bool READY_THREAD_GLOBAL_SYNC = false;

vector < int > clientsDescriptors;
int numberClientsDescriptors = 0;
bool numberClientsDescriptorsChang = false;

struct ClientSelectText CST[CLIENT_LIMIT];

void signal_callback_handler(int signum)
{
  cout << "#DEBUG: Signum = " << signum <<endl;
  end_program = true;
  READY_THREAD_GLOBAL_SYNC = true;
  cout << "#DEBUG: Start shutdown server" << endl;
}

void signal_callback_handler_PIPE(int signum)
{
    cout << "#ERROR: caught signal SIGPIPE " << signum << "!!!!!!" << endl;
}

int main()
{
    signal(SIGINT, signal_callback_handler);
    signal(SIGPIPE, signal_callback_handler_PIPE);
    //signal(SIGPIPE, SIG_IGN);

    for(int i = 0; i < PAGE_X; i++)
        for(int j = 0; j < PAGE_Y; j++)
            bufforFE[i][j] = '\0';

    for(int i = 0; i < CLIENT_LIMIT; i++) CST[i].descriptor = -1;

    thread acth(accept_connections_ed);
    thread cth(control_client);
    thread acath(accept_connections_activ);

    acth.join();
    acath.join();
    cth.join();

    cout << "#DEBUG: @@@@ EVERYTHING IS SUCCESSIVELY CLOSED @@@@" << endl;
    return 0;
}
