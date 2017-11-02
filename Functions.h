#include <fstream>
#include <vector>
#include "process.h.h"
#include <iostream>

using namespace std;


void readFile(vector<process> &p) {
    //ifstream inFile("process.txt");
    ifstream inFile("processes.txt");
    struct process pc;
    while (!inFile.eof()) {
        inFile >> pc.id >> pc.arrival >> pc.priority >> pc.runTime;
        p.push_back(pc);
    }
}