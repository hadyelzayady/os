#include "headers.h"
#include "process.h.h"
#include <iostream>
#include <fstream>

using namespace std;
int main(int argc, char* argv[]) {
//    ofstream of("temp.txt");
//    of.write("jwel",3);
    initClk();
    int rec_val;
    key_t rdyq = msgget(1, 0644 | IPC_CREAT);
    struct process p;
    rec_val = (int) msgrcv(1, &p, sizeof(p) - sizeof(long), 0, !IPC_NOWAIT);
    cout << "::" << rec_val << "sdsdsd";
    if (rec_val != -1) {
        if (atoi(argv[1]) == HPF) {
            cout << "HPF Algo\n";
        } else if (atoi(argv[1]) == RR) {
            cout << "RR algo\n";
        } else {// SRTF
            cout << "SRTF algo\n";
        }
    }

    //upon termination release clock
//    of.close();
    //  destroyClk(true);
//
//
}
