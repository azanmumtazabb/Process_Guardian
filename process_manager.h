
#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <sys/types.h>
#include <string>

namespace process_manager {

struct ProcessResult {
    bool success;
    int error_code;
    std::string message;
};

class ProcessManager {
public:
    static ProcessResult pauseProcess(pid_t pid);
    static ProcessResult resumeProcess(pid_t pid);
    static ProcessResult terminateProcess(pid_t pid);

    static ProcessResult setPriority(pid_t pid, int niceValue);
    static ProcessResult getPriority(pid_t pid, int& niceValue);

private:
    static ProcessResult sendSignal(
        pid_t pid, int signalNumber,
        const std::string& operation
    );

    static ProcessResult failure(
        const std::string& operation,
        int errorCode
    );
};

}

#endif
