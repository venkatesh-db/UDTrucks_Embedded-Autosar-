# CAN Protocol Working Demo

A simple, working CAN (Controller Area Network) protocol demonstration with Engine ECU and Instrument Cluster simulation.

## 🚗 What This Does

- **Engine ECU**: Sends realistic automotive data (RPM, Speed, Temperature, Fuel, Odometer)
- **Instrument Cluster**: Receives and displays CAN messages with validation
- **Real-time Communication**: Both sender and receiver work together seamlessly

## � Quick Start

### 1. Install Requirements
```bash
pip install python-can
```

### 2. Run the Working Demo
```bash
python3 can_demo.py
```

That's it! The demo will show:
- � Engine ECU sending realistic automotive data
- 📟 Instrument cluster receiving and validating messages
- 📊 Real-time dashboard updates every 20 messages

## 📊 Files in This Directory

```
canbasics/
├── can_demo.py            # ✅ WORKING CAN sender + receiver demo
├── setup_vcan.sh          # Virtual CAN setup script (optional)
└── README.md              # This file
```

## 🎛️ What You'll See

### Engine ECU Output (Sender):
```
📤 [15:57:10] Sent: RPM=1170 Speed=1km/h Temp=20.4°C Fuel=85% Odo=145230.0km
```

### Instrument Cluster Output (Receiver):
```
📥 [15:57:10] ✅ ENGINE_RPM: 1170.0 RPM
📥 [15:57:10] ✅ VEHICLE_SPEED: 1.0 km/h
📥 [15:57:10] 🥶 ENGINE_TEMP: 20.0 °C
📥 [15:57:10] ✅ FUEL_LEVEL: 85.0 %
📥 [15:57:10] ✅ ODOMETER: 145230.0 km
```

### Dashboard (Every 20 Messages):
```
� INSTRUMENT CLUSTER DASHBOARD
═══════════════════════════════
� Engine RPM:     1210 RPM
🏃 Vehicle Speed:   3.0 km/h  
🌡️  Engine Temp:    20.0 °C
⛽ Fuel Level:     85.0 %
🛣️  Odometer:   145230.0 km
═══════════════════════════════
```

## 🎯 Features Demonstrated

- **Realistic Engine Behavior**: Speed affects RPM, temperature warming, driving patterns
- **Proper CAN Encoding**: Binary message packing/unpacking
- **Data Validation**: Status indicators (✅ Normal, 🥶 Cold, 🔴 High, etc.)
- **Multi-threading**: Sender and receiver running simultaneously
- **Automotive CAN IDs**: Real automotive message IDs (0x0C4, 0x0B4, 0x1F0, etc.)

## 🚀 Usage

Simply run the demo and watch the realistic CAN communication:

```bash
python3 can_demo.py
```

Press `Ctrl+C` to stop.

## ✅ Why This Works

This demo uses a shared message queue instead of problematic virtual CAN interfaces, ensuring it works reliably on all systems (macOS, Linux, Windows) without complex setup.

Perfect for learning CAN protocol basics and automotive communication patterns! 🚗💨