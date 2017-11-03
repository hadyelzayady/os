#include <fstream>
#include <vector>
#include "process.h.h"
#include <iostream>
#include <string>
using namespace std;


void readFile(vector<process> &p) {
    ifstream inFile("processes.txt");
    string s;
    getline(inFile, s);
    struct process pc;
    while (inFile >> pc.id >> pc.arrival >> pc.runTime >> pc.priority) {
        p.push_back(pc);
    }
}