#include "alert_manager.h"
#include "logger.h"

#include <iomanip>
#include <sstream>

AlertManager::AlertManager(double cpuThreshold, double ramThresholdMb)
    : cpuThreshold_(cpuThreshold), ramThresholdMb_(ramThresholdMb) {}

void AlertManager::setCpuThreshold(double value) { cpuThreshold_ = value; }
void AlertManager::setRamThresholdMb(double value) { ramThresholdMb_ = value; }
double AlertManager::cpuThreshold() const { return cpuThreshold_; }
double AlertManager::ramThresholdMb() const { return ramThresholdMb_; }

std::vector<std::string> AlertManager::check(const std::vector<ProcessInfo>& processes) {
    std::vector<std::string> warnings;
    std::set<int> currentlyAlerted;

    for (const ProcessInfo& process : processes) {
        const double ramMb = process.memoryUsage / 1024.0;
        const bool highCpu = process.cpuUsage >= cpuThreshold_;
        const bool highRam = ramMb >= ramThresholdMb_;
        if (!highCpu && !highRam) continue;

        currentlyAlerted.insert(process.pid);
        std::ostringstream message;
        message << "Alert: PID " << process.pid << " (" << process.name << ") exceeds ";
        if (highCpu) message << "CPU " << std::fixed << std::setprecision(1)
                             << process.cpuUsage << "% (limit " << cpuThreshold_ << "%)";
        if (highCpu && highRam) message << ", ";
        if (highRam) message << "RAM " << std::fixed << std::setprecision(1)
                             << ramMb << " MB (limit " << ramThresholdMb_ << " MB)";
        warnings.push_back(message.str());
        if (alertedPids_.count(process.pid) == 0) Logger::log(message.str());
    }
    alertedPids_ = currentlyAlerted;
    return warnings;
}
