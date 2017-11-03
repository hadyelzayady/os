#include "headers.h"
#include "process.h.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

bool compare(const process &lhs, const process &rhs) {
        return lhs.runTime > rhs.runTime;
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
    if (p.stat == firstRun) {
        key_t pid = fork();
        if (pid == 0)//child
        {
            scheduler_log << "At time " << getClk() << " process " << p.id << " started arr " << p.arrival << " Total "
                          << p.runTime << " remain " << p.runTime << " wait 0" << endl;
            execl("./process.out", "process.out", to_string(p.runTime).c_str(), (char *) 0);
        } else {
            p.pid = pid;
        }
    } else {
        scheduler_log << "At time " << getClk() << " process " << p.id << " resumed arr " << p.arrival << " Total "
                      << p.runTime << " remain " << p.runTime << " wait " << endl;
        cerr << "continue process:";
        kill(p.pid, SIGCONT);//continue the process
    }
    p.stat = running;
}

void remove_process(int) {
    int stat_loc;
    int pid_wait = wait(&stat_loc);
    if (WIFEXITED(stat_loc))//exit code form child 1
    {
        process p = ps.back();
        scheduler_log << "At time " << getClk() << " process " << p.id << " finished arr " << p.arrival << " Total "
                      << p.runTime << " remain " << p.runTime << " wait 0" << endl;
        cout << "exit process with arrival:" << ps.back().arrival << " and id " << ps.back().id << endl;
        ps.pop_back();
        run_next();
    }
}

void stop_current() {
    process &p = ps.back();
    p.stat = paused;
    scheduler_log << "At time " << getClk() << " process " << p.id << " stopped arr " << p.arrival << " Total "
                  << p.runTime << " remain " << p.runTime << " wait 0" << endl;
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
    int rec_val = -1;
    key_t rdyq = msgget(1, 0644);
    key_t pid;
    int current_rt;
    int i = 0;
    signal(SIGCHLD, remove_process);
    signal(SIGUSR1, changeflag);
    int prevclk = 0;//to start loop firt time
    while (flag) {
        if (getClk() != prevclk) {
            struct process p;
            rec_val = (int) msgrcv(rdyq, &p, sizeof(p) - sizeof(long), 0, !IPC_NOWAIT);
            if (rec_val != -1) {
                cerr << "\nreceived\n" << getClk();/*<<p.runTime<<"  rem time in orev:"<<(getClk() - start_exec_time)*/;
                if (!ps.empty()) {
                    process &current_proc = ps.back();
                    current_proc.runTime = ps.back().runTime - (getClk() - start_exec_time);
                }
                if (ps.empty() || p.runTime < ps.back().runTime)//remaining time
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

                    } else {
                        scheduler_log << "At time " << getClk() << " process " << p.id << " started arr " << p.arrival
                                      << " Total " << p.runTime << " remain " << p.runTime << " wait 0" << endl;
                        execl("./process.out", "process.out", to_string(p.runTime).c_str(), (char *) 0);
                    }

                } else {
                    p.stat = firstRun;//not run yes;
                    ps.push_back(p);
                    sort(ps.begin(), ps.end(), compare);
                }

            } else {
            }
            prevclk = getClk();
        }

    }
    while (!ps.empty()) {}
    destroyClk(false);
    //upon termination release clock
//    of.close();
    scheduler_log.close();
    exit(0);


}
