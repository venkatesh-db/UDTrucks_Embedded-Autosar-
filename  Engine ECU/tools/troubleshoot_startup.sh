#!/bin/bash
# Engine ECU Startup Troubleshooting Script
# This script helps diagnose common ECU startup issues after software updates

echo "=========================================="
echo "Engine ECU Startup Troubleshooting Tool"
echo "=========================================="
echo ""

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print status
print_status() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✓${NC} $2"
    else
        echo -e "${RED}✗${NC} $2"
    fi
}

print_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

echo "1. Checking development environment..."

# Check if required tools are available
command -v python3 >/dev/null 2>&1
print_status $? "Python3 available for configuration analysis"

command -v canutils >/dev/null 2>&1 || command -v cansend >/dev/null 2>&1
print_status $? "CAN utilities available for bus testing"

echo ""
echo "2. Running AUTOSAR configuration analysis..."

# Run the configuration analyzer
if [ -f "tools/config_analyzer.py" ]; then
    python3 tools/config_analyzer.py config/
else
    print_warning "Configuration analyzer not found. Please run manually."
fi

echo ""
echo "3. Hardware connection checklist:"
echo "   □ Verify 12V power supply (±10% tolerance)"
echo "   □ Check ground connections"
echo "   □ Confirm CAN high/low wiring"
echo "   □ Validate termination resistors (120Ω each end)"
echo "   □ Test crystal oscillator (typically 8-16 MHz)"

echo ""
echo "4. Software verification steps:"
echo "   □ Confirm bootloader version compatibility"
echo "   □ Verify application signature/checksum"
echo "   □ Check memory layout (Flash/RAM sections)"
echo "   □ Validate stack size configuration"
echo "   □ Review interrupt vector table"

echo ""
echo "5. Communication interface tests:"
echo "   □ Monitor CAN bus activity with oscilloscope"
echo "   □ Check for CAN error frames"
echo "   □ Verify network management (NM) messages"
echo "   □ Test diagnostic communication (UDS)"

echo ""
echo "6. AUTOSAR stack validation:"
echo "   □ BSW (Basic Software) initialization sequence"
echo "   □ RTE (Runtime Environment) startup"
echo "   □ OS task creation and scheduling"
echo "   □ Software component initialization"

echo ""
echo "=========================================="
echo "Recommended Recovery Actions:"
echo "=========================================="
echo ""
echo "IMMEDIATE ACTIONS:"
echo "1. Power cycle the ECU (disconnect for 30 seconds)"
echo "2. Check for loose connections"
echo "3. Verify correct software version was flashed"
echo ""
echo "IF STILL NOT WORKING:"
echo "1. Try forcing ECU into bootloader/recovery mode"
echo "2. Re-flash the bootloader if suspected corruption"
echo "3. Restore previous working software version"
echo "4. Reset AUTOSAR configuration to defaults"
echo ""
echo "ADVANCED TROUBLESHOOTING:"
echo "1. Use JTAG/SWD debugger to check CPU state"
echo "2. Analyze memory dumps for corruption"
echo "3. Check watchdog configuration and timing"
echo "4. Review application startup sequence timing"
echo ""
echo "For additional support, check:"
echo "- README.md for detailed troubleshooting guide"
echo "- tools/diagnostic_checklist.md for systematic approach"
echo "- logs/startup_error_log_template.md to document findings"
echo ""