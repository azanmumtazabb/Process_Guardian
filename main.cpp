#include <iostream>
#include "process_manager.h"

using namespace std;
using namespace process_manager;

int main(int argc, char* argv[]) {

    if (argc != 2) {
        cout << "Usage: ./process_manager PID\n";
        return 1;
    }

    pid_t pid = stoi(argv[1]);
    int choice, niceValue;
    ProcessResult result;

    do {
        cout << "\n===== PROCESS MANAGER =====\n";
        cout << "1. Pause Process (SIGSTOP)\n";
        cout << "2. Resume Process (SIGCONT)\n";
        cout << "3. Terminate Process (SIGTERM)\n";
        cout << "4. Change Nice Value\n";
        cout << "5. Get Nice Value\n";
        cout << "0. Exit\n";
        cout << "Enter choice: ";

        cin >> choice;

        switch (choice) {
            case 1:
                result = ProcessManager::pauseProcess(pid);
                break;

            case 2:
                result = ProcessManager::resumeProcess(pid);
                break;

            case 3:
                result = ProcessManager::terminateProcess(pid);
                break;

            case 4:
                cout << "Enter nice value (-20 to 19): ";
                cin >> niceValue;
                result = ProcessManager::setPriority(pid, niceValue);
                break;

            case 5:
                result = ProcessManager::getPriority(pid, niceValue);
                if (result.success)
                    cout << "Current nice value: " << niceValue << endl;
                break;

            case 0:
                cout << "Exiting...\n";
                continue;

            default:
                cout << "Invalid choice!\n";
                continue;
        }

        cout << result.message << endl;

    } while (choice != 0);

    return 0;
}
