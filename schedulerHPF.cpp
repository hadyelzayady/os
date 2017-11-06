#include "headers.h"
#include "process.h.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
#include <algorithm>
#include <math.h>
#include <iomanip>

using namespace std;

bool compare_priority(const process &p1, const process &p2) {
    return p1.priority < p2.priority;
}

ofstream scheduler_log("scheduler.log");
vector<process> ps;
int start_time;
bool flag = true;// if process generator have procs to send ,flag is true
void run_next() {
    if (ps.empty())
        return;
    process &p = ps.back(); //get next process

    p.status = running;  // changing state
    p.waitingTime = getClk() - p.arrival;
    scheduler_log << "At time " << getClk() << " process " << p.id << " started arr " << p.arrival << " Total "
                  << p.runTime << " remain" << ' ' << p.runTime << " wait " << p.waitingTime << endl;
    start_time = getClk();
    key_t pid = fork();
    if (pid == 0) {
        execl("./process.out", "process.out", to_string(p.runTime).c_str(), (char *) 0);
    }

}

int TotalRT = 0;
double WTASum = 0;
int WaitintSum = 0;
int runtimeSum = 0;
int last_time;

vector<double> WTAS;

void Update_remaining_time(vector<process> &p) {
    for (int i = 0; i < p.size(); i++) {
        if (p[i].status == running) {
            p[i].remainTime = p[i].runTime - (getClk() - start_time); //getting the remaining time
        }
    }
}

void remove_process(int) {
    int stat_loc;
    int pid_wait = wait(&stat_loc);
    if (WIFEXITED(stat_loc))//exit code form child 1
    {
        process &p = ps.back();
        TotalRT += p.runTime;
        WTAS.push_back((double) (getClk() - p.arrival) / p.runTime);
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


void changeflag(int) {
    flag = false;
}


int main(int argc, char *argv[]) {
    initClk();
    int countofProc = 0;
    int rec_val = -1;
    key_t rdyq = msgget(1, 0644);
    key_t pid;
    int current_rt;
    int i = 0;
    bool handling_process = false;
    signal(SIGUSR2, remove_process);
    signal(SIGUSR1, changeflag);
    while (flag || rec_val != -1) {
        process p;
        rec_val = (int) msgrcv(rdyq, &p, sizeof(process) - sizeof(long), 0, !IPC_NOWAIT);
        if (rec_val != -1) {
            p.remainTime = p.runTime;
            p.waitingTime = 0;
            countofProc++;
            cerr << "\nreceived\n" << getClk();
            if (ps.empty()) {
                scheduler_log << "At time " << getClk() << " process " << p.id << " started arr " << p.arrival
                              << " Total "
                              << p.runTime << " remain" << p.runTime << " wait " << p.waitingTime << endl;
                start_time = getClk();
                key_t pid = fork();
                if (pid == 0) {
                    execl("./process.out", "process.out", to_string(p.runTime).c_str(), (char *) 0);
                } else {
                    p.status = running;
                    p.waitingTime = 0;
                    ps.push_back(p);
                }
            } else {
                p.status = waiting;
                process current = ps.back(); //getting current running process before sorting so not to lose it
                ps.pop_back();
                ps.push_back(p);
                sort(ps.begin(), ps.end(), compare_priority);
                ps.push_back(current); //returning the running process to vector
//                Update_remaining_time(ps); //update remaining time of running process;
                //cerr<<"after sort "<<ps.back().id<<endl;
            }
        }
    }
    Update_remaining_time(ps);
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
    destroyClk(false);
    //upon termination release clock
//    of.close();
    exit(0);

}
