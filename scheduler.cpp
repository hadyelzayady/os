#include "headers.h"
#include "process.h.h"
#include <iostream>
#include <fstream>
#include <queue>
#include <vector>
using namespace std;

struct LessRemainingRime {
    bool operator()(const process &lhs, const process &rhs) const {
        return lhs.runTime > rhs.runTime;
    }
};

priority_queue<process, vector<process>, LessRemainingRime> ps;
int start_exec_time = 0;
bool flag = true;// if process generator have procs to send ,flag is true
void run_next() {
    if (!flag)
        exit(0);
    if (ps.empty())
        return;
    struct process p = ps.top();
    ps.pop();
    start_exec_time = getClk();
    if (p.stat == firstRun) {
        key_t pid = fork();
        if (pid == 0)//child
            execl("./process.out", "process.out", to_string(p.runTime).c_str(), (char *) 0);
        else {
            p.id = pid;


        }
    } else {
        kill(p.id, SIGCONT);//continue the process
    }
    p.stat = running;
    ps.push(p);
}

void remove_process(int) {
    cout << "exit process:" << ps.top().id << endl;
    ps.pop();
    run_next();
}

void stop_current() {
    struct process p = ps.top();
    ps.pop();
    p.stat = paused;
    cout << "stop process: " << p.id << endl;
    ps.push(p);
    kill(p.id, SIGSTOP);


    //todo update remainging time
}

void changeflag(int) {
    flag = false;
}


int main(int argc, char* argv[]) {
    initClk();
    int rec_val = -1;
    key_t rdyq = msgget(1, 0644);
    int i = 0;
    signal(SIGCHLD, remove_process);
    signal(SIGCONT, changeflag);
    while (flag) {
        struct process p;
        rec_val = (int) msgrcv(rdyq, &p, sizeof(p) - sizeof(long), 0, !IPC_NOWAIT);
        if (rec_val != -1) {
            if (ps.empty() || p.runTime < (getClk() - start_exec_time))//remaining time
            {
                p.stat = running;//running
                if (!ps.empty())
                    stop_current();
                cout << "received\n";
                key_t pid = fork();
                if (pid == 0)//child
                {
                    execl("./process.out", "process.out", to_string(p.runTime).c_str(), (char *) 0);
                } else {
                    start_exec_time = getClk();
                    p.id = pid;
                    ps.push(p);


                }

            } else {
                p.stat = firstRun;//not run yes;
                ps.push(p);
            }

        } else {

            cout << flag << "error in receive\n";
        }

    }
    while (!ps.empty()) {}
    destroyClk(false);
    //upon termination release clock
//    of.close();
    exit(0);


}
