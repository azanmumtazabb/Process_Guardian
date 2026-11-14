#include "logger.h"
#include "user_interface.h"

int main() {
    Logger::log("Process Guardian started.");
    UserInterface interface;
    interface.run();
    return 0;
}
