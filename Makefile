CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic
LDFLAGS := -pthread -lncurses

SOURCES := main.cpp process_monitor.cpp process_manager.cpp user_interface.cpp alert_manager.cpp logger.cpp
TARGET := process_guardian

.PHONY: all monitor clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@ $(LDFLAGS)

monitor: test_main.cpp process_monitor.cpp
	$(CXX) $(CXXFLAGS) test_main.cpp process_monitor.cpp -o process_monitor -pthread

clean:
	rm -f $(TARGET) process_monitor process_guardian.log
