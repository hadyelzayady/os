#include <fstream>
#include <vector>
#include "process.h.h"
#include <iostream>
#include <string>
using namespace std;


void readFile(vector<process> &p) {
    //ifstream inFile("process.txt");
    ifstream inFile("processes.txt");
    string s;
    getline(inFile, s);
    struct process pc;
    while (inFile >> pc.id >> pc.arrival >> pc.runTime >> pc.priority) {
        pc.status = firstRun;
        pc.remainTime = pc.runTime;
        pc.pid = 1;
        p.push_back(pc);
    }
}