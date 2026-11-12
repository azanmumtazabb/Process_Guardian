#ifndef PROCESS_MONITOR_H
#define PROCESS_MONITOR_H

#include <string>
#include <vector>

using namespace std;

// Stores information about one process
struct ProcessInfo {

    int pid;
    string name;
    char state;

    double cpuUsage;
    double memoryUsage;

    int priority;
};

// Retrieves information about all processes
vector<ProcessInfo> getAllProcesses();

#endif
