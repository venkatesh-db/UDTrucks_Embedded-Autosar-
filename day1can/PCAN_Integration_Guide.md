
# PCAN Hardware Integration Instructions
# =====================================

## Hardware Setup:
1. Connect PCAN-USB device to computer
2. Install PCAN drivers from Peak System
3. Connect CAN-H and CAN-L to vehicle bus or test setup

## Software Installation:
```bash
# Install python-can with PCAN support
pip install python-can[pcan]

# Install PCAN-View (free monitoring tool)
# Download from: https://www.peak-system.com/PCAN-View.242.0.html
```

## Usage Example:
```python
import can

# Connect to PCAN hardware
bus = can.interface.Bus(bustype='pcan', 
                       channel='PCAN_USBBUS1', 
                       bitrate=500000)

# Send message
msg = can.Message(arbitration_id=0x123, data=[1, 2, 3, 4])
bus.send(msg)

# Receive messages
msg = bus.recv(timeout=1.0)
print(f"Received: {msg}")

bus.shutdown()
```

## Integration with Our Demos:
- Run real_can_integration.py with PCAN hardware connected
- Monitor traffic in PCAN-View while demos are running
- Export logs for professional analysis
