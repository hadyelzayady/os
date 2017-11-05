//
// Created by hady on 10/22/17.
//

#ifndef CODE_PROCESS_H_H
#define CODE_PROCESS_H_H
enum state {
    firstRun, running, waiting, finished
};
struct process {
    long pid;
    int id;
    int arrival;
    int runTime;
    int execTime;
    int remainTime;
    int waitingTime;
    int priority;
    int stop;
    state status;
};
enum algorithm {
    HPF = 1, RR = 2, SRTF = 3
};

#endif //CODE_PROCESS_H_H
