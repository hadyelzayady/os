#include "headers.h"
#include "process.h.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <math.h>
using namespace std;

bool compare(const process &lhs, const process &rhs) {
    return lhs.remainTime > rhs.remainTime;
    }

ofstream scheduler_log("scheduler.log");
vector<process> ps;
int start_exec_time = 0;
bool flag = true;// if process generator have procs to send ,flag is true
void run_next() {
    if (ps.empty())
        return;
    process &p = ps.back();
    start_exec_time = getClk();
    p.waitingTime += getClk() - p.stop;
    if (p.stat == firstRun) {
        key_t pid = fork();
        if (pid == 0)//child
        {
            execl("./process.out", "process.out", to_string(p.runTime).c_str(), (char *) 0);
        } else {
            scheduler_log << "At time " << getClk() << " process " << p.id << " started arr " << p.arrival << " Total "
                          << p.runTime << " remain " << p.remainTime << " wait " << p.waitingTime << endl;
            p.pid = pid;
        }
    } else {
        scheduler_log << "At time " << getClk() << " process " << p.id << " resumed arr " << p.arrival << " Total "
                      << p.runTime << " remain " << p.remainTime << " wait " << p.waitingTime << endl;
        cerr << "continue process:" << p.id;
        kill(p.pid, SIGCONT);//continue the process
    }
    p.stat = running;
}

int WTASum = 0;
int WaitintSum = 0;
void remove_process(int) {
    int stat_loc;
    int pid_wait = wait(&stat_loc);
    if (WIFEXITED(stat_loc))//exit code form child 1
    {
        process &p = ps.back();
        scheduler_log << "At time " << getClk() << " process " << p.id << " finished arr " << p.arrival << " Total "
                      << p.runTime << " remain 0" << " wait " << p.waitingTime << " TA " << getClk() - p.arrival
                      << " WTA " << (double) round((((double) (getClk() - p.arrival) / p.runTime) * 100)) / 100 << endl;
        WTASum += (double) (getClk() - p.arrival) / p.runTime;
        WaitintSum += p.waitingTime;
        cout << "exit process with arrival:" << ps.back().arrival << " and id " << ps.back().id << " TA "
             << getClk() - p.arrival << " WTA " << (double) (getClk() - p.arrival) / p.runTime << endl;
        ps.pop_back();
        run_next();
    }
}

void stop_current() {
    process &p = ps.back();
    p.stat = paused;
    p.stop = getClk();
    scheduler_log << "At time " << getClk() << " process " << p.id << " stopped arr " << p.arrival << " Total "
                  << p.runTime << " remain " << p.remainTime << " wait " << p.waitingTime << endl;
    cout << "stop process: " << p.id << endl;
    kill(p.pid, SIGSTOP);

    //todo update remainging time

}

void changeflag(int) {
    flag = false;
}

void write_log(int) {

}

int main(int argc, char* argv[]) {
    initClk();
    int countofProc = 0;
    int rec_val = -1;
    key_t rdyq = msgget(1, 0644);
    key_t pid;
    int current_rt;
    int i = 0;
    signal(SIGUSR2, remove_process);
    signal(SIGUSR1, changeflag);
    int prevclk = 0;//to start loop firt time
    while (flag) {
        process p;
        rec_val = (int) msgrcv(rdyq, &p, sizeof(p) - sizeof(long), 0, !IPC_NOWAIT);
        if (rec_val != -1) {
            p.remainTime = p.runTime;
            p.waitingTime = 0;
            countofProc++;
            cerr << "\nreceived\n" << getClk();/*<<p.runTime<<"  rem time in orev:"<<(getClk() - start_exec_time)*/;
            if (!ps.empty()) {
                process &current_proc = ps.back();
                current_proc.remainTime = ps.back().remainTime - (getClk() - start_exec_time);
            }
            if (ps.empty() || p.remainTime <
                              ps.back().remainTime)//remaining time,= as I do not know if sorting will put it in the back or before back
            {
                p.stat = running;//running
                if (!ps.empty())
                    stop_current();

                pid = fork();
                if (pid != 0)//scheduler
                {
                    start_exec_time = getClk();
                    p.pid = pid;
                    ps.push_back(p);
                    scheduler_log << "At time " << getClk() << " process " << p.id << " started arr " << p.arrival
                                  << " Total " << p.runTime << " remain " << p.remainTime << " wait 0" << endl;

                } else {

                    execl("./process.out", "process.out", to_string(p.runTime).c_str(), (char *) 0);
                }

            } else {
                if (p.remainTime == ps.back().remainTime) {
                    p.stop = getClk();
                    p.stat = firstRun;//not run yes;
                    //insert before the current proc because sort will put it u=in the back
                    process current = ps.back();
                    ps.pop_back();
                    ps.push_back(p);
                    ps.push_back(current);
                    cout << ps.back().id << endl;
                } else {
                    p.stop = getClk();
                    p.stat = firstRun;//not run yes;
                    ps.push_back(p);
                    sort(ps.begin(), ps.end(), compare);
                }
            }

        }
    }
    while (!ps.empty()) {}
    scheduler_log.close();
    ofstream scheduler_perform("scheduler_perform.txt");
    scheduler_perform << "CPU utilization=" << endl << "Avg WTA = " << (double) WTASum / countofProc << endl
                      << "Avg Waiting= " << (double) WaitintSum / countofProc << endl << "Std WTA=" << endl;
    scheduler_perform.close();
    destroyClk(false);
    //upon termination release clock
//    of.close();
    exit(0);


}
