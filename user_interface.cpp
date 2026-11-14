#include "user_interface.h"

#include "alert_manager.h"
#include "logger.h"
#include "process_manager.h"
#include "process_monitor.h"

#include <ncurses.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace {
std::string trimToWidth(const std::string& text, int width) {
    if (width <= 0) return "";
    if (static_cast<int>(text.size()) <= width) return text;
    return text.substr(0, std::max(0, width - 1)) + "~";
}

bool promptNumber(const char* label, double& value) {
    char input[32]{};
    timeout(-1);
    echo();
    curs_set(1);
    mvprintw(LINES - 2, 0, "%s", label);
    clrtoeol();
    getnstr(input, 31);
    noecho();
    curs_set(0);
    timeout(1000);
    try {
        std::size_t used = 0;
        value = std::stod(input, &used);
        return used == std::string(input).size();
    } catch (...) { return false; }
}

void logAction(const std::string& action, int pid, const process_manager::ProcessResult& result) {
    Logger::log(action + " PID " + std::to_string(pid) + ": " + result.message);
}
}

void UserInterface::run() {
    initscr(); cbreak(); noecho(); keypad(stdscr, TRUE); curs_set(0); timeout(1000);
    start_color(); use_default_colors();
    init_pair(1, COLOR_CYAN, -1); init_pair(2, COLOR_RED, -1); init_pair(3, COLOR_YELLOW, -1);

    AlertManager alerts;
    std::vector<ProcessInfo> processes;
    std::vector<std::string> warnings;
    int selected = 0;
    std::string status = "Press r to load processes.";
    bool running = true;

    while (running) {
        if (processes.empty()) {
            processes = getAllProcesses();
            warnings = alerts.check(processes);
            selected = 0;
            status = "Process list refreshed.";
        }
        erase();
        attron(COLOR_PAIR(1) | A_BOLD);
        mvprintw(0, 0, "Process Guardian - Smart Process Resource Manager");
        attroff(COLOR_PAIR(1) | A_BOLD);
        mvprintw(1, 0, "CPU alert: %.1f%%   RAM alert: %.1f MB   Log: %s", alerts.cpuThreshold(), alerts.ramThresholdMb(), Logger::fileName());
        mvprintw(2, 0, "Up/Down select | r refresh | c CPU limit | m RAM limit | p pause | s resume | t terminate | n nice | q quit");
        attron(A_BOLD);
        mvprintw(4, 0, "%-7s %-20s %-7s %-9s %-9s %-10s", "PID", "NAME", "STATE", "PRIORITY", "CPU %", "RAM MB");
        attroff(A_BOLD);
        mvhline(5, 0, ACS_HLINE, COLS);

        const int visibleRows = std::max(1, LINES - 10);
        const int first = std::max(0, selected - visibleRows + 1);
        for (int row = 0; row < visibleRows && first + row < static_cast<int>(processes.size()); ++row) {
            const ProcessInfo& p = processes[first + row];
            if (first + row == selected) attron(A_REVERSE);
            mvprintw(6 + row, 0, "%-7d %-20s %-7c %-9d %-9.2f %-10.2f", p.pid,
                     trimToWidth(p.name, 20).c_str(), p.state, p.priority, p.cpuUsage, p.memoryUsage / 1024.0);
            if (first + row == selected) attroff(A_REVERSE);
        }

        const int footer = LINES - 3;
        if (!warnings.empty()) { attron(COLOR_PAIR(2) | A_BOLD); mvprintw(footer, 0, "%s", trimToWidth(warnings.front(), COLS).c_str()); attroff(COLOR_PAIR(2) | A_BOLD); }
        else { attron(COLOR_PAIR(3)); mvprintw(footer, 0, "No active threshold alerts."); attroff(COLOR_PAIR(3)); }
        mvprintw(LINES - 1, 0, "%s", trimToWidth(status, COLS).c_str());
        refresh();

        const int key = getch();
        if (key == ERR) {
            processes = getAllProcesses();
            warnings = alerts.check(processes);
            selected = std::min(selected, std::max(0, static_cast<int>(processes.size()) - 1));
            status = "Process list refreshed automatically.";
            continue;
        }
        if (key == 'q' || key == 'Q') { running = false; continue; }
        if (key == KEY_UP && selected > 0) --selected;
        else if (key == KEY_DOWN && selected + 1 < static_cast<int>(processes.size())) ++selected;
        else if (key == 'r' || key == 'R') { processes = getAllProcesses(); warnings = alerts.check(processes); selected = std::min(selected, std::max(0, static_cast<int>(processes.size()) - 1)); status = "Process list refreshed."; }
        else if (key == 'c' || key == 'C') { double v; if (promptNumber("New CPU threshold (0-100): ", v) && std::isfinite(v) && v >= 0 && v <= 100) { alerts.setCpuThreshold(v); Logger::log("CPU threshold changed to " + std::to_string(v) + "%"); status = "CPU threshold updated."; } else status = "Invalid CPU threshold."; }
        else if (key == 'm' || key == 'M') { double v; if (promptNumber("New RAM threshold in MB (> 0): ", v) && std::isfinite(v) && v > 0) { alerts.setRamThresholdMb(v); Logger::log("RAM threshold changed to " + std::to_string(v) + " MB"); status = "RAM threshold updated."; } else status = "Invalid RAM threshold."; }
        else if (!processes.empty() && (key == 'p' || key == 'P' || key == 's' || key == 'S' || key == 't' || key == 'T')) {
            const int pid = processes[selected].pid;
            process_manager::ProcessResult result = key == 'p' || key == 'P' ? process_manager::ProcessManager::pauseProcess(pid) : key == 's' || key == 'S' ? process_manager::ProcessManager::resumeProcess(pid) : process_manager::ProcessManager::terminateProcess(pid);
            const std::string action = (key == 'p' || key == 'P') ? "Paused" : (key == 's' || key == 'S') ? "Resumed" : "Terminated";
            logAction(action, pid, result); status = action + " PID " + std::to_string(pid) + ": " + result.message;
        }
        else if (!processes.empty() && (key == 'n' || key == 'N')) { double v; if (promptNumber("New nice value (-20 to 19): ", v) && v >= -20 && v <= 19 && v == static_cast<int>(v)) { int pid = processes[selected].pid; auto result = process_manager::ProcessManager::setPriority(pid, static_cast<int>(v)); logAction("Changed nice value for", pid, result); status = result.message; } else status = "Nice value must be an integer from -20 to 19."; }
        else status = "Unknown command.";
    }
    Logger::log("Process Guardian closed.");
    endwin();
}
