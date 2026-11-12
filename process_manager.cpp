
#include "process_manager.h"

#include <iostream>
#include <cerrno>
#include <cstring>
#include <signal.h>
#include <sys/resource.h>
#include <unistd.h>

namespace process_manager {

ProcessResult ProcessManager::failure(
    const std::string& operation, int errorCode) {

    return {
        false,
        errorCode,
        operation + " failed: " + std::strerror(errorCode)
    };
}

ProcessResult ProcessManager::sendSignal(
    pid_t pid, int signalNumber,
    const std::string& operation) {

    if (pid <= 0)
        return failure(operation, EINVAL);

    if (kill(pid, signalNumber) == -1) {
        int savedError = errno;
        return failure(operation, savedError);
    }

    return {true, 0, operation + " successful."};
}

ProcessResult ProcessManager::pauseProcess(pid_t pid) {
    return sendSignal(pid, SIGSTOP, "Pause process");
}

ProcessResult ProcessManager::resumeProcess(pid_t pid) {
    return sendSignal(pid, SIGCONT, "Resume process");
}

ProcessResult ProcessManager::terminateProcess(pid_t pid) {
    return sendSignal(pid, SIGTERM, "Terminate process");
}

ProcessResult ProcessManager::setPriority(
    pid_t pid, int niceValue) {

    if (pid <= 0 || niceValue < -20 || niceValue > 19)
        return failure("Set priority", EINVAL);

    if (setpriority(PRIO_PROCESS, pid, niceValue) == -1) {
        int savedError = errno;
        return failure("Set priority", savedError);
    }

    return {true, 0, "Priority changed successfully."};
}

ProcessResult ProcessManager::getPriority(
    pid_t pid, int& niceValue) {

    if (pid <= 0)
        return failure("Get priority", EINVAL);

    errno = 0;
    int result = getpriority(PRIO_PROCESS, pid);

    if (result == -1 && errno != 0) {
        int savedError = errno;
        return failure("Get priority", savedError);
    }

    niceValue = result;

    return {true, 0, "Priority retrieved successfully."};
}

}
