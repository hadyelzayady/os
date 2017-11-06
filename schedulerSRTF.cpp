#include "headers.h"
#include "process.h.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <math.h>
#include <iomanip>      // std::setprecision
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
    if (p.status == firstRun) {
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
    p.status = running;
}

int TotalRT = 0;
double WTASum = 0;
int WaitintSum = 0;
vector<double> WTAS;
void remove_process(int) {
    int stat_loc;
    int pid_wait = wait(&stat_loc);
    if (WIFEXITED(stat_loc))//exit code form child 1
    {
        process &p = ps.back();
        TotalRT += p.runTime;
        WTAS.push_back((double) round((((double) (getClk() - p.arrival) / p.runTime) * 100)) / 100);
        scheduler_log << fixed << setprecision(2) << "At time " << getClk() << " process " << p.id << " finished arr "
                      << p.arrival << " Total "
                      << p.runTime << " remain 0" << " wait " << p.waitingTime << " TA " << getClk() - p.arrival
                      << " WTA " << ((getClk() - p.arrival) / (double) p.runTime) << endl;
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
    p.status = waiting;
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

void start(int) {
    kill(getppid(), SIGURG);

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
    signal(SIGUNUSED, start);
    int prevclk = 0;//to start loop firt time
    int sizem = sizeof(process) - sizeof(long);
    while (flag) {
        vector<process> pp;
        process p;
        int i = 0, index;
        int clk = getClk();
        while (clk == getClk() && msgrcv(rdyq, &p, sizem, 0, !IPC_NOWAIT) != -1) {
            p.stop = clk;
            p.status = firstRun;
            pp.push_back(p);
            cerr << p.id << endl;
            cerr << "sending conf\n" << clk << " " << p.id << endl;

        }
        cerr << "out of loop\n";
        int firstwithus = 0;
        if (pp.size() != 0) {
            p.waitingTime = 0;
            countofProc += pp.size();
            cerr << "\nreceived\n" << getClk();/*<<p.runTime<<"  rem time in orev:"<<(getClk() - start_exec_time)*/;
            if (!ps.empty()) {
                process &current_proc = ps.back();
                current_proc.remainTime = ps.back().remainTime - (getClk() - start_exec_time);
            }
            if (ps.empty() || pp[0].runTime <
                              ps.back().remainTime)//remaining time,= as I do not know if sorting will put it in the back or before back
            {
                firstwithus += 1;
                pp[0].status = running;//running
                pp[0].stop = 0;
                if (!ps.empty())
                    stop_current();

                pid = fork();
                if (pid != 0)//scheduler
                {
                    start_exec_time = getClk();
                    pp[0].pid = pid;
                    ps.push_back(pp[0]);
                    scheduler_log << "At time " << getClk() << " process " << pp[0].id << " started arr "
                                  << pp[0].arrival
                                  << " Total " << pp[0].runTime << " remain " << pp[0].remainTime
                                  << " wait 0" << endl;


                } else {

                    execl("./process.out", "process.out", to_string(p.runTime).c_str(), (char *) 0);
                }

            } else {
                    //insert before the current proc because sort will put it u=in the back
                ps.insert(ps.begin(), pp.begin() + firstwithus, pp.end());
                sort(ps.begin(), ps.end() - 1, compare);
                }
            }
    }

    while (!ps.empty()) {}
    scheduler_log.close();
    ofstream scheduler_perform("scheduler.perf");
    double AWTA = (double) WTASum / countofProc;
    cout << AWTA;
    scheduler_perform << "CPU utilization=" << fixed << setprecision(2) << ((double) TotalRT / getClk()) * 100 << "%"
                      << endl << "Avg WTA = " << AWTA << endl
                      << "Avg Waiting= " << (double) WaitintSum / countofProc << endl;
    double sum = 0;
    for (int j = 0; j < WTAS.size(); ++j) {
        sum += (WTAS[i] - AWTA) * (WTAS[i] - AWTA);
    }

    scheduler_perform << "Std WTA=" << sqrt((double) sum / countofProc);
    scheduler_perform.close();
    destroyClk(true);
    //upon termination release clock
//    of.close();
    exit(0);


}
