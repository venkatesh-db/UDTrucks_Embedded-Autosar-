# AUTOSAR Infotainment ECU Battery Drain Case Study

## Build Instructions

### Prerequisites
- C++11 compatible compiler (GCC 7+ or Clang 6+)
- Make or CMake (optional)
- Git (for version control)

### Quick Build
```bash
cd "/Users/venkatesh/Embedded Autosar /Infotainment ECU"

# Simple build with g++
g++ -std=c++11 -o main main.cpp \
    src/PowerManager/PowerManager.cpp \
    src/InfotainmentSystem/InfotainmentSystem.cpp \
    src/Diagnostics/PowerMonitor.cpp \
    -I. -pthread -O2

# Run the case study
./main
```

### Usage Examples

```bash
# Interactive mode (default)
./main

# Run vehicle operation simulation
./main simulation

# Run all battery drain scenarios  
./main scenarios

# Run real-time power dashboard
./main dashboard
```

## Makefile

```makefile
CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra -O2 -I.
LDFLAGS = -pthread

SRCDIR = src
SOURCES = main.cpp \
          $(SRCDIR)/PowerManager/PowerManager.cpp \
          $(SRCDIR)/InfotainmentSystem/InfotainmentSystem.cpp \
          $(SRCDIR)/Diagnostics/PowerMonitor.cpp

TARGET = autosar_battery_drain_case_study

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET) *.csv *.log

run: $(TARGET)
	./$(TARGET)

scenarios: $(TARGET)
	./$(TARGET) scenarios

simulation: $(TARGET)
	./$(TARGET) simulation

dashboard: $(TARGET) 
	./$(TARGET) dashboard

.PHONY: all clean run scenarios simulation dashboard
```

Save this as `Makefile` and then run:
```bash
make           # Build
make run       # Run interactive mode
make scenarios # Run battery drain scenarios
make dashboard # Run power dashboard
```