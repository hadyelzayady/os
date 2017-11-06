#include "headers.h"
#include "process.h.h"
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <iomanip>
#include <math.h>

using namespace std;
int pid; //should it be inside;
int flag = 0;      //indication if there are other processes to send;
int num = 0;
int cnt;    //count used for synchronization
int ts;     //T switch;
int qtm = 0;        //actual qtm;


void changeflag(int) {
    cout << "at time " << getClk() << " flag 1" << endl;
    flag = 1;
}


vector<process> proctable;   //process table;
vector<float> wta;           //weighted turn around;
vector<int> wt;//waiting time;
ofstream scheduler_log("scheduler.log");

int rt = 0;  //all runtimes;

int findind(int pid) {
    for (int i = 0; i < proctable.size(); i++) {
        if (proctable[i].pid == pid) return i;
    }
    return -1;
}


void chgstatus(int pid, bool type)  //type=0--->run else waiting
{
    int place = findind(pid);
    if (!type) proctable[place].status = running;
    else {
        proctable[place].status = waiting;
        if (proctable[place].waitingTime < 0) proctable[place].waitingTime = -1 * proctable[place].waitingTime;
    }
}

void chgtime()    //decrease remaining time || increase waiting time
{    //cout<<"chg "<<getClk()<<"size "<<proctable.size()<<endl;
    for (int i = 0; i < proctable.size(); i++) {
        //cout<<"id "<<proctable[i].id<<endl;
        if (proctable[i].status == running) {
            proctable[i].remainTime--;
            if (proctable[i].remainTime < 0) proctable[i].remainTime = 0;
            //cout<<"id rem "<<proctable[i].id<<endl;
        } else {
            proctable[i].waitingTime++;
            if (proctable[i].waitingTime < 0) proctable[i].waitingTime = -1 * proctable[i].waitingTime;
            //cout<<proctable[i].id<<"is waiting"<<proctable[i].waitingTime<<endl;
        }
    }

}

void procfinished(int) {
    int st;
    int pid = wait(&st);
    int place = findind(pid);
    if (proctable[place].waitingTime < 0) proctable[place].waitingTime = -1 * proctable[place].waitingTime;
    while (getClk() < ts + cnt && qtm <= 1) {}
    scheduler_log << fixed << setprecision(2) << "At time " << getClk() << " process " << proctable[place].id
                  << " Finished arr " << proctable[place].arrival << " Total "
                  << proctable[place].runTime << " remain " << proctable[place].remainTime << " wait "
                  << proctable[place].waitingTime << " TA " << getClk() - proctable[place].arrival << " WTA "
                  << ((getClk() - proctable[place].arrival) / ((double) proctable[place].runTime)) << endl;

    rt += proctable[place].runTime;
    wt.push_back(proctable[place].waitingTime);
    wta.push_back(((getClk() - proctable[place].arrival) / ((double) proctable[place].runTime)));
    num++;
    proctable.erase(proctable.begin() + place);


}



int main(int argc, char *argv[]) {
    // sleep(1);
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

    int remstat = 1;   //process needs another quantum?

    int rdyq;
    string remtime;
    struct process message;
    bool firstread = false;

    //sleep(1.5);

    //TODO: implement the scheduler :)

    rdyq = msgget(1, 0644 | IPC_CREAT);

    while (1)    //another check should be done related to quantum (running variable)
    {

        time = getClk();
        if (qtm == 0) {
            ts = time;

            qtm = quantum;
            cnt = qtm;
            remstat = 1;
            if (flag == 1 && firstread) {
                int stat1 = msgrcv(rdyq, &message, sizeof(message) - sizeof(long), 0, IPC_NOWAIT);
                //cout<<"Flag="<<flag<<" stat="<<stat1<<endl;
                if (stat1 == -1) {
                    cout << "I will go" << endl;
                    break;
                }
            }
            if (flag == 0 || !firstread) {//cout<<"Sch before 2"<<endl;
                //firstread=true;
                //
                stat = msgrcv(rdyq, &message, sizeof(message) - sizeof(long), 0,
                              IPC_NOWAIT);   //should it be IPC_NOWAIT?;
            }
            if (stat == -1) qtm = 0;
            //and here
            if (stat != -1) {
                //cout<<"First"<<endl;
                firstread = true;
                cout << "Time " << time << " process " << message.id << " RT " << message.runTime << " rem " << remstat
                     << endl;
                if (message.remainTime <= quantum) {
                    qtm = message.remainTime;
                    cnt = qtm;
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
                    scheduler_log << "At time " << getClk() << " process " << message.id << " started arr "
                                  << message.arrival << " Total "
                                  << message.runTime << " remain " << message.runTime << " wait "
                                  << getClk() - message.arrival << endl;
                    chgstatus(message.pid, 0);
                    if (remstat != 0) {

                        message.status = running;
                        chgstatus(message.pid, 0);

                    }
                } else {
                    message.status = running;
                    chgstatus(message.pid, 0);
                    scheduler_log << "At time " << getClk() << " process " << message.id << " resumed arr  "
                                  << message.arrival << " Total "
                                  << message.runTime << " remain " << message.remainTime << " wait "
                                  << proctable[findind(message.pid)].waitingTime << endl;

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
                    while (getClk() < ts + cnt && qtm == 0) {}
                }

                if (qtm == 0 && remstat != 0) {

                    cout << "sch will send at time " << time << endl;
                    message.status = waiting;
                    chgstatus(message.pid, 1);
                    message.remainTime -= quantum;
                    scheduler_log << "At time " << getClk() << " process " << message.id << " stopped arr "
                                  << message.arrival << " Total "
                                  << message.runTime << " remain " << message.remainTime << " wait "
                                  << proctable[findind(message.pid)].waitingTime << endl;

                    int y = msgsnd(rdyq, &message, sizeof(message) - sizeof(long), !IPC_NOWAIT);
                    cout << "send stat " << y << "process " << message.id << endl;
                    kill(message.pid, SIGSTOP);
                }
            }      //here
        }


    }                 //while(1) bracket;
    cout << "bye sch" << endl;
    sleep(1);
    ofstream scheduler_perform("scheduler_perform.txt");
    double clck = (double) getClk();
    uti = (double) rt / clck;
    //uti = round(uti * 100) / 100;
    uti = uti * 100;     //utilization percentage;
    float avwta = 0;
    float avwt = 0;
    for (int i = 0; i < wta.size(); i++) {
        avwt += wt[i];
        avwta += wta[i];
    }

    avwt = avwt / (float) num;
    //avwt = round(avwt * 100) / 100;    //avg waiting;
    avwta = avwta / (float) num;
    //avwta = round(avwta * 100) / 100;  //average turnaround
    float mid = 0;  //intermediate term;
    for (int i = 0; i < wta.size(); i++) {
        mid += (wta[i] - avwta) * (wta[i] - avwta);
    }

    scheduler_log.close();

    float stdv = sqrt(mid / (float) num);
    //stdv=round(stdv*100)/100;

    scheduler_perform << fixed << setprecision(2) << "CPU utilization=" << uti << endl << "Avg WTA = " << avwta << endl
                      << "Avg Waiting= " << avwt << endl << "Std WTA=" << stdv << endl;


    scheduler_perform.close();
    destroyClk(true);
    exit(0);
    //upon termination release clock


}