#include "main.hpp"


void signal_callback_handler(int signum)
{
    cout << "#DEBUG: Signum = " << signum <<endl;
    //START CLOSING SERVER
}


int main()
{
    signal(SIGINT, signal_callback_handler);
    signal(SIGPIPE, SIG_IGN);

    cout << "#DEBUG: @@@@ SERVER STARTED @@@@" << endl;

    cout << "#DEBUG: @@@@ SERVER IS SUCCESSIVELY CLOSED @@@@" << endl;
    return 0;
}
