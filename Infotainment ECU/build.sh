#!/bin/bash

# AUTOSAR Infotainment ECU Battery Drain Case Study Build Script
echo "=== Building AUTOSAR Infotainment ECU Case Study ==="

# Create build directory if it doesn't exist
mkdir -p build

# Compile the application
echo "Compiling source files..."

# C++ compiler flags
CXX_FLAGS="-std=c++11 -Wall -Wextra -O2 -pthread"
INCLUDE_DIRS="-I."
OUTPUT="build/battery_drain_case_study"

# Source files
SOURCES=(
    "main.cpp"
    "src/PowerManager/PowerManager.cpp"
    "src/InfotainmentSystem/InfotainmentSystem.cpp" 
    "src/Diagnostics/PowerMonitor.cpp"
)

# Check if all source files exist
for source in "${SOURCES[@]}"; do
    if [ ! -f "$source" ]; then
        echo "Error: Source file $source not found!"
        exit 1
    fi
done

# Compile command
compile_cmd="g++ $CXX_FLAGS $INCLUDE_DIRS -o $OUTPUT ${SOURCES[*]}"

echo "Build command: $compile_cmd"
echo ""

# Execute compilation
if $compile_cmd; then
    echo ""
    echo "✅ Build successful!"
    echo "Executable created: $OUTPUT"
    echo ""
    echo "Usage examples:"
    echo "  ./$OUTPUT                    # Interactive mode"
    echo "  ./$OUTPUT scenarios          # Run battery drain scenarios"
    echo "  ./$OUTPUT dashboard          # Real-time power dashboard"
    echo "  ./$OUTPUT simulation         # Vehicle operation simulation"
    echo "  ./$OUTPUT help               # Show help"
    echo ""
    echo "To run directly: cd build && ./battery_drain_case_study"
else
    echo ""
    echo "❌ Build failed!"
    echo "Please check the error messages above."
    exit 1
fi