#include "headers.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "process.h.h"
#include "Functions.h"
#include <sys/msg.h>
#include <string>
#include <algorithm>
using namespace std;
void ClearResources(int);

bool compareSRTF(const process &p1, const process &p2) {
    return p1.runTime < p2.runTime && p1.arrival == p2.arrival;
}

bool compareHPF(const process &p1, const process &p2) {
    return p1.priority > p2.priority && p1.arrival == p2.arrival;
}


key_t rdyq;

int main() {
//

    //TODO:
    // 1-Ask the user about the chosen scheduling Algorithm and its parameters if exists.
    cout << "Enter desired algorithm(HPF,RR,SRTN,Exit)->(1,2,3,0): ";
    int algo;
    cin >> algo;
    if (algo > 3 || algo <= 0)
        return 0;
    string quantom;
    vector<process> processesData;
    readFile(processesData);
    if (algo == RR) {
        cout << "enter quantum: ";
        cin >> quantom;
        while (atoi(quantom.c_str()) < 1) {
            cout << "enter quantum: ";
            cin >> quantom;
        }
    } else if (algo == HPF) {
        sort(processesData.begin(), processesData.end(), compareHPF);
    } else {
        sort(processesData.begin(), processesData.end(), compareSRTF);
    }
    signal(SIGINT, ClearResources);
    signal(SIGCHLD, ClearResources);
    rdyq = msgget(1, 0644 | IPC_CREAT);       //initializing ready queue;
    int pidclk = fork();
    if (pidclk == 0) {

        execl("./clock.out", "clock.out", NULL);


    }
    int pidsch;
    if (algo == HPF) {
        pidsch = fork();
        if (pidsch == 0)
            execl("./schHPF.out", "schHPF.out", NULL);

    } else if (algo == RR) {

        pidsch = fork();
        if (pidsch == 0) {
            execl("./schRR.out", "schRR.out", quantom.c_str(), (char *) 0);
        }
    } else {
        pidsch = fork();
        if (pidsch == 0)
            execl("./schSRTF.out", "schSRTF.out", NULL);
    }

    initClk();
    int x = getClk();
    printf("current time is %d\n", x);

    int firstArriveIndex = 0;
    while (firstArriveIndex < processesData.size()) {
        while (firstArriveIndex < processesData.size() &&
               processesData[firstArriveIndex].arrival <= getClk()) {
            int send = msgsnd(rdyq, &processesData[firstArriveIndex],
                              sizeof(process) - sizeof(long),
                              !IPC_NOWAIT);
            cerr << processesData[firstArriveIndex].id << endl;
            cerr << getClk() << endl;
            if (send == -1)
                cout << "error in sending\n";
            firstArriveIndex++;

        }
    }


    kill(pidsch, SIGUSR1);
    while (1) {}


}

void ClearResources(int)
{
    msgctl(rdyq, IPC_RMID, (struct msqid_ds *) 0);
    destroyClk(true);
    exit(0);


}



