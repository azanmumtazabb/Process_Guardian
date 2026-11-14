#include "logger.h"

#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <mutex>

namespace {
std::mutex logMutex;
const char* const LOG_FILE = "process_guardian.log";
}

const char* Logger::fileName() {
    return LOG_FILE;
}

void Logger::log(const std::string& event) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::ofstream file(LOG_FILE, std::ios::app);
    if (!file) return;

    const std::time_t now = std::time(nullptr);
    std::tm localTime{};
    localtime_r(&now, &localTime);
    file << '[' << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S")
         << "] " << event << '\n';
}
