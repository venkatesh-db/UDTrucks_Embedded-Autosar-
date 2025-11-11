#!/bin/bash
# QA Engineer Validation Script
# This is what QA engineers actually need to run and observe

echo "🔍 QA ENGINEER VALIDATION CHECKLIST"
echo "===================================="

# Test 1: Build Validation
echo ""
echo "TEST 1: BUILD VALIDATION"
echo "Status: " 
make clean && make
BUILD_STATUS=$?
if [ $BUILD_STATUS -eq 0 ]; then
    echo "✅ BUILD: PASSED"
else
    echo "❌ BUILD: FAILED - CRITICAL ISSUE"
    exit 1
fi

# Test 2: Runtime Validation  
echo ""
echo "TEST 2: RUNTIME VALIDATION"
echo "Status: "
./engine_ecu > runtime_output.log 2>&1
RUNTIME_STATUS=$?
if [ $RUNTIME_STATUS -eq 0 ]; then
    echo "✅ RUNTIME: PASSED"
else
    echo "❌ RUNTIME: FAILED - CRITICAL ISSUE"
    exit 1
fi

# Test 3: Error Count Validation
echo ""
echo "TEST 3: ERROR COUNT VALIDATION"
ERROR_COUNT=$(grep "Error Count:" runtime_output.log | grep -o "[0-9]*")
if [ "$ERROR_COUNT" = "0" ]; then
    echo "✅ ERROR COUNT: PASSED (0 errors)"
else
    echo "❌ ERROR COUNT: FAILED ($ERROR_COUNT errors found)"
    exit 1
fi

# Test 4: Success Message Validation
echo ""
echo "TEST 4: SUCCESS MESSAGE VALIDATION"
if grep -q "ENGINE ECU STARTUP SUCCESSFUL" runtime_output.log; then
    echo "✅ SUCCESS MESSAGE: PASSED"
else
    echo "❌ SUCCESS MESSAGE: FAILED - No success confirmation"
    exit 1
fi

# Test 5: Configuration Validation
echo ""
echo "TEST 5: CONFIGURATION VALIDATION"
python3 tools/config_analyzer.py config/ > config_output.log 2>&1
if grep -q "No configuration issues detected" config_output.log; then
    echo "✅ CONFIGURATION: PASSED"
else
    echo "❌ CONFIGURATION: FAILED - Issues detected"
    exit 1
fi

echo ""
echo "🎉 ALL QA TESTS PASSED - SYSTEM APPROVED FOR RELEASE!"
echo "====================================================="

# Cleanup
rm -f runtime_output.log config_output.log