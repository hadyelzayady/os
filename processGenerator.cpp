#include "headers.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "process.h.h"
#include "Functions.h"
#include <sys/msg.h>

using namespace std;
void ClearResources(int);


key_t rdyq;

int main() {

     signal(SIGINT,ClearResources);

    rdyq = msgget(1, 0644 | IPC_CREAT);       //initializing ready queue;
    //TODO:
    // 1-Ask the user about the chosen scheduling Algorithm and its parameters if exists.
    cout << "Enter desired algorithm(RR,HPF,SRTN,Exit)->(1,2,3,0): ";
    int algo;
    cin >> algo;
//     2-Initiate and create Scheduler and Clock processes.
//     3-use this function after creating clock process to initialize clock
    int pidclk = fork();

    if (pidclk == 0) {
        execl("./clock.out", "clock.out", NULL);
    } else {
//        int pidsch=fork();
//        if(pidsch==0)
//        {
//            execl("./sch.out","sch.out",NULL);
//        }
//        else{
        initClk();
        int x = getClk();
        printf("current time is %d\n", x);
        vector<process> processesData;
        readFile(processesData);
        int firstArriveIndex = 0;
        while (firstArriveIndex < processesData.size()) {
            while (processesData[firstArriveIndex].arrival <= getClk()) {
                msgsnd(rdyq, &processesData[firstArriveIndex], sizeof(processesData[firstArriveIndex]) - sizeof(long),
                       !IPC_NOWAIT);
                firstArriveIndex++;

            }

            destroyClk(true);

        }
    }
//
//
//    }

    ///Toget time use the following function

//    TODO:  Generation Main Loop
//    4-Creating a data structure for process  and  provide it with its parameters
//    5-Send the information to  the scheduler at the appropriate time
//    (only when a process arrives) so that it will be put it in its turn.


//    6-clear clock resources


}

void ClearResources(int)
{
    //TODO: it clears all resources in case of interruption
    msgctl(rdyq, IPC_RMID, (struct msqid_ds *) 0);
}




