
#include "headers.h"
#include <signal.h>
#include <iostream>

using namespace std;
/* Modify this file as needed*/
int remainingtime =0;

//void pauseHandler(int) {
//    cout << "stop process: " << getpid() << endl;
//
//    pause();
//}

int main(int agrc, char* argv[]) {

    //if you need to use the emulated clock uncomment the following line
    //initClk();
    cout << "process started:" << getpid() << endl;
    //TODO: it needs to get the remaining time from somewhere
    remainingtime = atoi(argv[1]);
    while(remainingtime>0) {
        cout << "process: " << getpid() << "  " << remainingtime << endl;
        sleep(1);
        remainingtime--;
    }
    kill(getppid(), SIGUSR2);
    exit(getpid());
    //if you need to use the emulated clock uncomment the following line
    //destroyClk(false);
}
