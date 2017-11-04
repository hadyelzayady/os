#include "headers.h"
#include <iostream>
#include <vector>
#include <fstream>
#include "process.h.h"
#include "Functions.h"
#include <sys/msg.h>
#include <stdio.h>
#include <string>

using namespace std;
void ClearResources(int);

void childhandler(int) {
    int y;
    wait(&y);
    if (!WIFEXITED(y)) perror("problem");
    if (WIFSIGNALED(y)) cout << WTERMSIG(y) << endl;
    int time = getClk();
    cout << "Sch terminated at time " << time << endl;
    ClearResources(0);

}

key_t rdyq;

int main() {

    signal(SIGINT, ClearResources);
    signal(SIGCHLD, childhandler);
    rdyq = msgget(1, 0644 | IPC_CREAT);       //initializing ready queue;
    //cout<<"rdyq is "<<rdyq;
    //sleep(3);
    //TODO:
    // 1-Ask the user about the chosen scheduling Algorithm and its parameters if exists.
    cout << "Enter desired algorithm(RR,HPF,SRTN,Exit)->(1,2,3,0): ";
    int algo;
    int q = 4;
    string qntm;
    cin >> algo;
    //   if (algo==0) ClearResources(1);
    if (algo == 1) //R.R
    {
        cout << "Enter quantum :" << endl;
        cin >> q;
        while (q < 1) {
            cout << "Error enter a valid value for q > 0 :" << endl;
            cin >> q;
        }
        qntm = std::to_string(q);
    }
//     2-Initiate and create Scheduler and Clock processes.
//     3-use this function after creating clock process to initialize clock
    int pidclk = fork();

    if (pidclk == 0) {   //cout<<"I am the clock starting"<<endl;
        execl("./clock.out", "clock.out", NULL);
    } else {
        initClk();
        int pidsch = fork();
        if (pidsch == 0) {
            execl("./schRR.out", "schRR.out", qntm.c_str(), NULL);
            //execl("./schRR.out","schRR.out",NULL);
        } else {
            //sleep(0.5);
            //initClk();
            //sleep(10);
            int x = getClk();
            printf("current time is %d\n", x);
            vector<process> processesData;
            readFile(processesData);
            cout << "read from file" << endl;
            for (int i = 0; i < processesData.size(); i++) {
                // processesData[i].mtype=0;
                processesData[i].status = firstRun;
            }
            int firstArriveIndex = 0;
            //processesData.size();
            while (firstArriveIndex < processesData.size()) {
                //cout<<"clk "<<getClk()<<" index "<<firstArriveIndex<<endl;

                while (firstArriveIndex < processesData.size() && processesData[firstArriveIndex].arrival <= getClk()) {
                    //cout<<"new "<<rdyq<<endl;
                    int x = msgsnd(rdyq, &processesData[firstArriveIndex],
                                   sizeof(processesData[firstArriveIndex]) - sizeof(long),
                                   !IPC_NOWAIT);
                    cout << "prcgen sending " << x << endl;

                    firstArriveIndex++;
                    //sleep(2);

                }
            }


            struct process pro;
            pro.runTime = -1;
            int z = msgsnd(rdyq, &pro, sizeof(pro) - sizeof(long), !IPC_NOWAIT);
            if (z == -1) perror("error in send term");
            cout << "main is finishing with stat " << z << endl;

        }
    }
    while (1) {/*cout<<"while1"<<endl;*/};
//
}

///Toget time use the following function

//    TODO:  Generation Main Loop
//    4-Creating a data structure for process  and  provide it with its parameters
//    5-Send the information to  the scheduler at the appropriate time
//    (only when a process arrives) so that it will be put it in its turn.


//    6-clear clock resources


//}
//}

void ClearResources(int x)    //sighandler of interrupt //x=0 ---->deleting from sch
{
    //TODO: it clears all resources in case of interruption
    msgctl(rdyq, IPC_RMID, (struct msqid_ds *) 0);
    if (x == 0) destroyClk(true);
    exit(0);

}




