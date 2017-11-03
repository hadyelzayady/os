//
// Created by hady on 10/22/17.
//

#ifndef CODE_PROCESS_H_H
#define CODE_PROCESS_H_H
enum status {
    running, firstRun, paused
};
struct process {
    long id;
    int arrival;
    int runTime;
    int priority;
    int stat;
};
enum algorithm {
    HPF = 1, RR, SRTF
};

#endif //CODE_PROCESS_H_H
