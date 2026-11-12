
#include "process_monitor.h"

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using namespace std;

int main()
{
    // Retrieve all available processes
    vector<ProcessInfo> processes = getAllProcesses();

    // Display program heading
    cout << "\n========== PROCESS GUARDIAN ==========\n\n";

    // Display table headings
    cout << left
         << setw(10) << "PID"
         << setw(25) << "PROCESS NAME"
         << setw(10) << "STATE"
         << setw(12) << "PRIORITY"
	 << setw(12) << "CPU (%)"
         << setw(15) << "RAM (KB)"
         << endl;

    cout << string(84, '-') << endl;

    // Display information about each process
    for (const auto& process : processes)
    {
        cout << left
             << setw(10) << process.pid
             << setw(25) << process.name
             << setw(10) << process.state
             << setw(12) << process.priority
             << fixed << setprecision(2)
	     << setw(12) << process.cpuUsage
             << process.memoryUsage
             << endl;
    }

    // Display total number of processes
    cout << "\nTotal Processes Found: "
         << processes.size() << endl;

    return 0;
}

