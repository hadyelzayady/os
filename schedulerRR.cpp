#include "headers.h"
#include "process.h.h"
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>

using namespace std;
int pid; //should it be inside;

int main(int argc, char *argv[]) {
    signal(SIGUSR2, SIG_IGN);
    cout << "Hello from sch" << endl;
    int time;
    int stat;   //status of receiving from process generator;
    int quantum = atoi(argv[1]);  //constant qtm;
    //int quantum=4;
    cout << "Qntm " << quantum << endl;
    int qtm = 0;        //actual qtm;
    int remstat = 1;   //process needs another quantum?
    int flag = 0;      //indication if there are other processes to send;
    int rdyq;
    string remtime;
    struct process message;


    initClk();

    //TODO: implement the scheduler :)
    sleep(0.5);
    time = getClk();
    int prevtime = time;
    rdyq = msgget(1, 0644 | IPC_CREAT);

    //cout<<"sch rdyq "<<rdyq<<endl;
    while (1)    //another check should be done related to quantum (running variable)
    {

        time = getClk();

        //cout<<"Sch before"<<endl;
        //cout<<"qtm "<<qtm<<" stat "<<stat<<" time "<<time<<endl;

        //sleep(2);

        if (qtm == 0) {
            qtm = quantum;
            remstat = 1;
            if (flag == 1) {
                int stat1 = msgrcv(rdyq, &message, sizeof(message) - sizeof(long), 0, IPC_NOWAIT);
                //cout<<"Flag="<<flag<<" stat="<<stat1<<endl;
                if (stat1 == -1) break;
            }
            // if(flag==0)
            // {
            if (flag == 0) {//cout<<"Sch before 2"<<endl;
                stat = msgrcv(rdyq, &message, sizeof(message) - sizeof(long), 0,
                              IPC_NOWAIT);   //should it be IPC_NOWAIT?;
                //cout<<"sch receiving "<<stat<<endl;
            }
            if (stat == -1) qtm = 0;
            if (stat != -1) {    //here
                if (message.runTime == -1) {
                    flag = 1;
                    qtm = 0;
                }
            }       //and here
            if (stat != -1 && message.runTime != -1) {
                //if(message.runTime>quantum) //should we add status of process (blocked etc in it)
                // { message.runtime-=quantum;
                // msgsnd(rdyq,&message, sizeof(message)-sizeof(long),!IPC_NOWAIT);
                //}
                cout << "Time " << time << " process " << message.id << " RT " << message.runTime << " rem " << remstat
                     << endl;
                if (message.runTime <= quantum) {
                    qtm = message.runTime;
                    remstat = 0;

                    cout << "Anti Riri" << endl;
                }


                if (message.status == firstRun) //running for first time;
                {
                    pid = fork();
                    if (pid == 0) {
                        remtime = std::to_string(message.runTime);
                        execl("./process.out", "process.out", remtime.c_str(), NULL);  //child=process
                    }
                    message.pid = pid;//parent

                    if (remstat != 0) message.status = running;
                } else {
                    cout << " Running p " << message.id << " at time " << time << " qtm " << qtm << endl;
                    kill(message.pid, SIGCONT);

                }

            }
            //cout<<"Iam out"<<qtm<<endl;
        } else {   //cout<<"prev "<<prevtime<<endl;
            if (stat != -1) {    //here   //qtm!=0 -----> process running;
                if (prevtime != time) {   //cout<<"the else"<<endl;
                    qtm--;
                    cout << "qtm " << qtm << endl;
                    prevtime = time;
                }
                //cout<<"here qtm "<<qtm<<" remstat "<<remstat<<endl;
                if (qtm == 0 && remstat != 0) {
                    cout << "sch will send at time " << time << endl;
                    message.runTime -= quantum;
                    int y = msgsnd(rdyq, &message, sizeof(message) - sizeof(long), !IPC_NOWAIT);
                    cout << "send stat " << y << "process " << message.id << endl;
                    kill(message.pid, SIGSTOP);
                }
            }      //here
        }


    }                 //while(1) bracket;
    cout << "bye sch" << endl;
    destroyClk(true);
    exit(0);
    //upon termination release clock


}