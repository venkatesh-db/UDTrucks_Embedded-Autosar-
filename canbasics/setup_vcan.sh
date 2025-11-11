#!/bin/bash

# Virtual CAN Setup Script for macOS/Linux
# This script sets up virtual CAN interfaces for testing

echo "🔧 Setting up Virtual CAN interface..."

# Check if running on macOS or Linux
if [[ "$OSTYPE" == "darwin"* ]]; then
    echo "📱 Detected macOS - Using python-can virtual interface"
    echo "Note: On macOS, we'll use python-can's built-in virtual bus"
    echo "No additional setup required for virtual CAN on macOS"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    echo "🐧 Detected Linux - Setting up vcan interface"
    
    # Load the vcan module
    sudo modprobe vcan
    
    # Create virtual CAN interface
    sudo ip link add dev vcan0 type vcan
    sudo ip link set up vcan0
    
    echo "✅ Virtual CAN interface 'vcan0' created and activated"
    echo "You can view it with: ip link show vcan0"
else
    echo "❌ Unsupported OS type: $OSTYPE"
    exit 1
fi

echo "🚀 Virtual CAN setup complete!"
echo "You can now run the CAN protocol scripts."