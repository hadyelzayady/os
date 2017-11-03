#include <fstream>
#include <vector>
#include "process.h.h"
#include <iostream>
#include <string>
using namespace std;


void readFile(vector<process> &p) {
    //ifstream inFile("process.txt");
    ifstream inFile("processes.txt");
    struct process pc;
    inFile >> pc.id >> pc.arrival >> pc.runTime >> pc.priority;
    while (!inFile.eof()) {
        p.push_back(pc);
        inFile >> pc.id >> pc.arrival >> pc.runTime >> pc.priority;
    }
}