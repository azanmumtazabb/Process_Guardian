#ifndef ALERT_MANAGER_H
#define ALERT_MANAGER_H

#include "process_monitor.h"

#include <set>
#include <string>
#include <vector>

class AlertManager {
public:
    AlertManager(double cpuThreshold = 80.0, double ramThresholdMb = 500.0);

    void setCpuThreshold(double value);
    void setRamThresholdMb(double value);
    double cpuThreshold() const;
    double ramThresholdMb() const;

    std::vector<std::string> check(const std::vector<ProcessInfo>& processes);

private:
    double cpuThreshold_;
    double ramThresholdMb_;
    std::set<int> alertedPids_;
};

#endif
