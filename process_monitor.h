#ifndef PROCESS_MONITOR_H
#define PROCESS_MONITOR_H

#include <string>
#include <vector>

// Stores information about one process
struct ProcessInfo {

    int pid;
    std::string name;
    char state;

    double cpuUsage;
    double memoryUsage;

    int priority;
};

// Retrieves information about all processes
std::vector<ProcessInfo> getAllProcesses();

#endif
