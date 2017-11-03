//
// Created by hady on 10/22/17.
//

#ifndef CODE_PROCESS_H_H
#define CODE_PROCESS_H_H
enum status {
    running, firstRun, paused
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
    status stat;
};
enum algorithm {
    HPF = 1, RR, SRTF
};

#endif //CODE_PROCESS_H_H
