#include "headers.h"
#include "process.h.h"
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <math.h>

using namespace std;
int pid; //should it be inside;
int flag = 0;      //indication if there are other processes to send;
int num = 0;
void changeflag(int) {
    flag = 1;
}


vector<process> proctable;   //process table;
vector<float> wta;           //weighted turn around;
vector<int> wt;//waiting time;

int rt = 0;  //all runtimes;

int findind(int pid) {
    for (int i = 0; i < proctable.size(); i++) {
        if (proctable[i].pid == pid) return i;
    }
    return -1;
}


void chgstatus(int pid, bool type)  //type=0--->run else paused
{
    int place = findind(pid);
    if (!type) proctable[place].status = running;
    else proctable[place].status = paused;
}

void chgtime()    //decrease remaining time || increase waiting time
{
    for (int i = 0; i < proctable.size(); i++) {
        if (proctable[i].status == running) proctable[i].remainTime--;
        else proctable[i].waitingTime++;
    }

}

void procfinished(int) {
    int st;
    int pid = wait(&st);
    int place = findind(pid);
    rt += proctable[place].runTime;
    wt.push_back(proctable[place].waitingTime);
    wta.push_back(proctable[place].waitingTime / proctable[place].runTime);
    num++;
    proctable.erase(proctable.begin() + place);


}



int main(int argc, char *argv[]) {
    sleep(1);
    initClk();
    int time;
    float uti;
    time = getClk();
    int prevtime = time;
    signal(SIGUSR2, procfinished);
    signal(SIGUSR1, changeflag);
    cout << "Hello from sch" << endl;
    int stat;   //status of receiving from process generator;
    int quantum = atoi(argv[1]);  //constant qtm;
    cout << "Qntm " << quantum << endl;
    int qtm = 0;        //actual qtm;
    int remstat = 1;   //process needs another quantum?

    int rdyq;
    string remtime;
    struct process message;

    //sleep(1.5);

    //TODO: implement the scheduler :)

    rdyq = msgget(1, 0644 | IPC_CREAT);

    while (1)    //another check should be done related to quantum (running variable)
    {

        time = getClk();
        if (qtm == 0) {
            qtm = quantum;
            remstat = 1;
            if (flag == 1) {
                int stat1 = msgrcv(rdyq, &message, sizeof(message) - sizeof(long), 0, IPC_NOWAIT);
                //cout<<"Flag="<<flag<<" stat="<<stat1<<endl;
                if (stat1 == -1) break;
            }
            if (flag == 0) {//cout<<"Sch before 2"<<endl;
                stat = msgrcv(rdyq, &message, sizeof(message) - sizeof(long), 0,
                              IPC_NOWAIT);   //should it be IPC_NOWAIT?;
            }
            if (stat == -1) qtm = 0;
            //and here
            if (stat != -1) {
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
                    message.execTime = getClk();
                    message.waitingTime = message.arrival - getClk();
                    proctable.push_back(message);

                    if (remstat != 0) {

                        message.status = running;
                        chgstatus(message.pid, 0);

                    }
                } else {
                    chgstatus(message.pid, 0);
                    cout << " Running p " << message.id << " at time " << time << " qtm " << qtm << endl;
                    kill(message.pid, SIGCONT);

                }

            }
        } else {   //cout<<"prev "<<prevtime<<endl;
            if (stat != -1) {    //here   //qtm!=0 -----> process running;
                if (prevtime != time) {   //cout<<"the else"<<endl;
                    chgtime();
                    qtm--;
                    cout << "qtm " << qtm << endl;
                    prevtime = time;
                }
                if (qtm == 0 && remstat != 0) {
                    cout << "sch will send at time " << time << endl;
                    message.status = paused;
                    chgstatus(message.pid, 1);
                    message.runTime -= quantum;
                    int y = msgsnd(rdyq, &message, sizeof(message) - sizeof(long), !IPC_NOWAIT);
                    cout << "send stat " << y << "process " << message.id << endl;
                    kill(message.pid, SIGSTOP);
                }
            }      //here
        }


    }                 //while(1) bracket;
    cout << "bye sch" << endl;
    double clck = (double) getClk();
    uti = (double) rt / clck;
    uti = round(uti * 100) / 100;
    uti = uti * 100;     //utilization percentage;
    float avwta = 0;
    float avwt = 0;
    for (int i = 0; i < wta.size(); i++) {
        avwt += wt[i];
        avwta += wta[i];
    }

    avwt = avwt / (float) num;
    avwt = round(avwt * 100) / 100;    //avg waiting;
    avwta = avwta / (float) num;
    avwta = round(avwta * 100) / 100;  //average turnaround

    destroyClk(true);
    exit(0);
    //upon termination release clock


}