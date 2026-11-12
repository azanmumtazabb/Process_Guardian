
#include "process_monitor.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <dirent.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cctype>
#include <cstdlib>
#include <cerrno>
#include <map>
#include <thread>
#include <chrono>

using namespace std;

// Stores CPU information for one process between samples
struct ProcessSample
{
    unsigned long long cpuTicks;
    unsigned long long startTime;
};

// Checks whether a directory name contains only digits
bool isNumeric(const string& name)
{
    if (name.empty())
        return false;

    return all_of(name.begin(), name.end(),
        [](unsigned char c) {
            return isdigit(c);
        });
}

// Reads process information from /proc/[PID]/stat
bool readProcessData(
    const string& directoryName,
    char& processState,
    int& priority,
    unsigned long long& cpuTicks,
    unsigned long long& startTime)
{
    string filePath =
        "/proc/" + directoryName + "/stat";

    ifstream processFile(filePath);

    if (!processFile.is_open())
        return false;

    string line;

    if (!getline(processFile, line))
        return false;

    // Find the first '(' and last ')'
    // This handles process names containing spaces.
    size_t firstBracket = line.find('(');
    size_t lastBracket = line.rfind(')');

    if (firstBracket == string::npos ||
        lastBracket == string::npos ||
        lastBracket <= firstBracket)
        return false;

    // Extract fields after the process name
    string remainingFields =
        line.substr(lastBracket + 2);

    istringstream stream(remainingFields);

    vector<string> fields;
    string field;

    while (stream >> field)
    {
        fields.push_back(field);
    }

    // Check that all required fields are available
    if (fields.size() <= 19)
        return false;

    try
    {
        // Field 3: Process state
        processState = fields[0][0];

        // Field 14: User CPU time
        unsigned long long userTime =
            stoull(fields[11]);

        // Field 15: Kernel CPU time
        unsigned long long systemTime =
            stoull(fields[12]);

        // Field 18: Process priority
        priority = stoi(fields[15]);

        // Field 22: Process start time
        startTime = stoull(fields[19]);

        // Total CPU time used by this process
        cpuTicks = userTime + systemTime;
    }
    catch (...)
    {
        return false;
    }

    return true;
}

// Reads total CPU time from /proc/stat
bool readTotalCpuTime(unsigned long long& totalTime)
{
    ifstream cpuFile("/proc/stat");

    if (!cpuFile.is_open())
        return false;

    string cpuLabel;

    if (!(cpuFile >> cpuLabel) || cpuLabel != "cpu")
        return false;

    totalTime = 0;

    // Sum the first eight CPU counters.
    // Guest counters are excluded because they are
    // already included in user and nice time.
    for (int i = 0; i < 8; i++)
    {
        unsigned long long value;

        if (!(cpuFile >> value))
            return false;

        totalTime += value;
    }

    return true;
}

// Reads RAM usage from /proc/[PID]/status
bool readMemoryUsage(
    const string& directoryName,
    double& memoryUsage)
{
    string filePath =
        "/proc/" + directoryName + "/status";

    ifstream processFile(filePath);

    if (!processFile.is_open())
        return false;

    string line;

    while (getline(processFile, line))
    {
        // Find the VmRSS entry
        if (line.find("VmRSS:") == 0)
        {
            istringstream stream(line);

            string memoryLabel;
            double memoryValue;
            string memoryUnit;

            if (stream >> memoryLabel
                       >> memoryValue
                       >> memoryUnit)
            {
                memoryUsage = memoryValue;
                return true;
            }
        }
    }

    return false;
}

// Retrieves information about all accessible processes
vector<ProcessInfo> getAllProcesses()
{
    vector<ProcessInfo> processes;

    // Store the first CPU sample for each process
    map<int, ProcessSample> firstSamples;

    unsigned long long totalCpuBefore = 0;
    unsigned long long totalCpuAfter = 0;

    // Take the first system CPU reading
    if (!readTotalCpuTime(totalCpuBefore))
    {
        cerr << "Error: Unable to read /proc/stat.\n";
        return processes;
    }

    // Open the /proc directory
    DIR* procDir = opendir("/proc");

    if (procDir == nullptr)
    {
        cerr << "Error: Unable to open /proc directory.\n";
        return processes;
    }

    struct dirent* entry;

    // Read all entries inside /proc
    while ((entry = readdir(procDir)) != nullptr)
    {
        string directoryName = entry->d_name;

        // Skip entries that are not numeric PIDs
        if (!isNumeric(directoryName))
            continue;

        // Convert directory name into an integer PID
        errno = 0;
        char* end;

        long pidValue = strtol(
            directoryName.c_str(), &end, 10
        );

        if (errno != 0 ||
            *end != '\0' ||
            pidValue <= 0 ||
            pidValue > 2147483647)
        {
            continue;
        }

        int pid = static_cast<int>(pidValue);

        // Read process name from /proc/[PID]/comm
        string filePath =
            "/proc/" + directoryName + "/comm";

        ifstream processFile(filePath);

        if (!processFile.is_open())
            continue;

        string processName;

        if (!getline(processFile, processName))
            continue;

        // Create a process record
        ProcessInfo process{};

        process.pid = pid;
        process.name = processName;
        process.state = '?';
        process.cpuUsage = 0.0;
        process.memoryUsage = 0.0;
        process.priority = 0;

        unsigned long long processCpuTicks = 0;
        unsigned long long processStartTime = 0;

        // Read process state, priority and CPU counters
        bool dataRead = readProcessData(
            directoryName,
            process.state,
            process.priority,
            processCpuTicks,
            processStartTime
        );

        if (dataRead)
        {
            firstSamples[pid] = {
                processCpuTicks,
                processStartTime
            };
        }

        // Read process RAM usage
        readMemoryUsage(
            directoryName,
            process.memoryUsage
        );

        // Add process information to the vector
        processes.push_back(process);
    }

    // Close /proc directory
    closedir(procDir);

    // Wait 500 milliseconds before taking the second sample
    this_thread::sleep_for(
        chrono::milliseconds(500)
    );

    // Take the second system CPU reading
    if (!readTotalCpuTime(totalCpuAfter) ||
        totalCpuAfter <= totalCpuBefore)
    {
        cerr << "Warning: Unable to calculate CPU usage.\n";
        return processes;
    }

    // Calculate total CPU time difference
    unsigned long long totalCpuDifference =
        totalCpuAfter - totalCpuBefore;

    // Take the second reading for every process
    for (auto& process : processes)
    {
        auto sample = firstSamples.find(process.pid);

        if (sample == firstSamples.end())
            continue;

        char currentState = '?';
        int currentPriority = 0;

        unsigned long long currentCpuTicks = 0;
        unsigned long long currentStartTime = 0;

        string directoryName = to_string(process.pid);

        bool dataRead = readProcessData(
            directoryName,
            currentState,
            currentPriority,
            currentCpuTicks,
            currentStartTime
        );

        if (!dataRead)
            continue;

        // Check that the PID still belongs to the same process.
        // This prevents incorrect calculations if a PID is reused.
        if (currentStartTime != sample->second.startTime)
            continue;

        if (currentCpuTicks < sample->second.cpuTicks)
            continue;

        unsigned long long processCpuDifference =
            currentCpuTicks - sample->second.cpuTicks;

        // Calculate CPU usage as a percentage of total
        // machine CPU capacity.
        process.cpuUsage =
            (static_cast<double>(processCpuDifference) /
             static_cast<double>(totalCpuDifference)) * 100.0;
    }

    // Sort processes by PID
    sort(processes.begin(), processes.end(),
        [](const ProcessInfo& a, const ProcessInfo& b) {
            return a.pid < b.pid;
        });

    return processes;
}
