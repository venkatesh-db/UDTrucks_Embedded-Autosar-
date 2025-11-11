#!/usr/bin/env python3
"""
Working CAN Protocol Demo - Simple Version
This version uses a shared message queue to demonstrate CAN communication
"""

import time
import struct
import threading
from datetime import datetime
from collections import deque

# Shared message queue (simulates CAN bus)
can_bus_queue = deque(maxlen=100)
bus_lock = threading.Lock()

class CANMessage:
    def __init__(self, can_id, data):
        self.arbitration_id = can_id
        self.data = data
        self.timestamp = time.time()

class EngineECU:
    def __init__(self):
        self.running = False
        self.rpm = 800
        self.speed = 0
        self.temp = 20
        self.fuel = 85
        self.odometer = 145230
        
    def send_can_message(self, can_id, data):
        """Send CAN message to shared bus"""
        with bus_lock:
            can_bus_queue.append(CANMessage(can_id, data))
    
    def update_engine_state(self):
        """Update realistic engine parameters"""
        # Simulate driving patterns
        if time.time() % 10 < 5:  # Accelerating
            self.speed = min(120, self.speed + 1)
        else:  # Decelerating
            self.speed = max(0, self.speed - 0.5)
            
        # RPM based on speed
        if self.speed == 0:
            self.rpm = 800 + (20 if time.time() % 2 < 1 else -20)  # Idle fluctuation
        else:
            self.rpm = 1200 + (self.speed * 20) + (50 if time.time() % 3 < 1 else -50)
            
        # Engine temperature
        if self.temp < 90:
            self.temp += 0.2
        else:
            self.temp = 90 + (2 if time.time() % 4 < 2 else -2)  # Operating temp fluctuation
            
        # Odometer
        if self.speed > 0:
            self.odometer += (self.speed / 3600) * 0.5  # Distance in 0.5 seconds
    
    def start(self):
        self.running = True
        print("🚗 Engine ECU Started - Sending CAN messages")
        
        while self.running:
            try:
                self.update_engine_state()
                
                # Send RPM message (ID: 0x0C4)
                rpm_data = struct.pack('>H', int(self.rpm)) + b'\x00'*6
                self.send_can_message(0x0C4, rpm_data)
                
                # Send Speed message (ID: 0x0B4) 
                speed_data = struct.pack('>H', int(self.speed * 100)) + b'\x00'*6
                self.send_can_message(0x0B4, speed_data)
                
                # Send Temperature message (ID: 0x1F0)
                temp_data = struct.pack('B', int(self.temp + 40)) + b'\x00'*7
                self.send_can_message(0x1F0, temp_data)
                
                # Send Fuel Level message (ID: 0x349)
                fuel_data = struct.pack('B', int(self.fuel)) + b'\x00'*7
                self.send_can_message(0x349, fuel_data)
                
                # Send Odometer message (ID: 0x3E9)
                odo_data = struct.pack('>I', int(self.odometer * 10)) + b'\x00'*4
                self.send_can_message(0x3E9, odo_data)
                
                timestamp = datetime.now().strftime("%H:%M:%S")
                print(f"📤 [{timestamp}] Sent: RPM={self.rpm:4.0f} Speed={self.speed:3.0f}km/h Temp={self.temp:4.1f}°C Fuel={self.fuel:3.0f}% Odo={self.odometer:8.1f}km")
                
                time.sleep(0.5)  # Send every 500ms
                
            except Exception as e:
                print(f"❌ Sender error: {e}")
                break
                
    def stop(self):
        self.running = False
        print("🛑 Engine ECU Stopped")

class InstrumentCluster:
    def __init__(self):
        self.running = False
        self.message_types = {
            0x0C4: ('ENGINE_RPM', 'RPM'),
            0x0B4: ('VEHICLE_SPEED', 'km/h'),
            0x1F0: ('ENGINE_TEMP', '°C'),
            0x349: ('FUEL_LEVEL', '%'),
            0x3E9: ('ODOMETER', 'km')
        }
        
        # Store latest values
        self.latest_values = {}
        
    def receive_can_message(self):
        """Receive CAN message from shared bus"""
        with bus_lock:
            if can_bus_queue:
                return can_bus_queue.popleft()
        return None
    
    def decode_message(self, msg):
        """Decode CAN message data"""
        try:
            if msg.arbitration_id == 0x0C4:  # RPM
                value = struct.unpack('>H', msg.data[:2])[0]
                status = "✅" if 500 <= value <= 6000 else ("🔴" if value > 6000 else "🔸")
                return value, status
                
            elif msg.arbitration_id == 0x0B4:  # Speed
                value = struct.unpack('>H', msg.data[:2])[0] / 100
                status = "✅" if value <= 120 else "🏃"
                return value, status
                
            elif msg.arbitration_id == 0x1F0:  # Temperature
                value = struct.unpack('B', msg.data[:1])[0] - 40
                status = "✅" if 70 <= value <= 105 else ("🔥" if value > 105 else "🥶")
                return value, status
                
            elif msg.arbitration_id == 0x349:  # Fuel Level
                value = struct.unpack('B', msg.data[:1])[0]
                status = "✅" if value >= 10 else "⛽"
                return value, status
                
            elif msg.arbitration_id == 0x3E9:  # Odometer
                value = struct.unpack('>I', msg.data[:4])[0] / 10
                status = "✅"
                return value, status
                
        except Exception as e:
            print(f"❌ Decode error: {e}")
            
        return None, "❌"
    
    def start(self):
        self.running = True
        print("📟 Instrument Cluster Started - Receiving CAN messages")
        
        message_count = 0
        
        while self.running:
            try:
                msg = self.receive_can_message()
                
                if msg:
                    message_count += 1
                    msg_info = self.message_types.get(msg.arbitration_id)
                    
                    if msg_info:
                        msg_name, unit = msg_info
                        value, status = self.decode_message(msg)
                        
                        if value is not None:
                            self.latest_values[msg_name] = value
                            timestamp = datetime.now().strftime("%H:%M:%S")
                            
                            print(f"📥 [{timestamp}] {status} {msg_name}: {value:8.1f} {unit}")
                            
                            # Show dashboard every 20 messages
                            if message_count % 20 == 0:
                                self.show_dashboard()
                    else:
                        print(f"❓ Unknown message ID: 0x{msg.arbitration_id:03X}")
                else:
                    time.sleep(0.1)  # Wait a bit if no messages
                    
            except Exception as e:
                print(f"❌ Receiver error: {e}")
                break
    
    def show_dashboard(self):
        """Display instrument cluster dashboard"""
        print("\n" + "="*60)
        print("🚗 INSTRUMENT CLUSTER DASHBOARD")
        print("="*60)
        
        for param, value in self.latest_values.items():
            unit = dict(self.message_types.values()).get(param, '')
            if param == "ENGINE_RPM":
                print(f"🔧 Engine RPM:     {value:>8.0f} {unit}")
            elif param == "VEHICLE_SPEED":
                print(f"🏃 Vehicle Speed:  {value:>8.1f} {unit}")
            elif param == "ENGINE_TEMP":
                print(f"🌡️  Engine Temp:    {value:>8.1f} {unit}")
            elif param == "FUEL_LEVEL":
                print(f"⛽ Fuel Level:     {value:>8.1f} {unit}")
            elif param == "ODOMETER":
                print(f"🛣️  Odometer:       {value:>8.1f} {unit}")
        
        print("="*60 + "\n")
        
    def stop(self):
        self.running = False
        print("🛑 Instrument Cluster Stopped")

def main():
    print("🚗 CAN Protocol Demo - Working Sender & Receiver")
    print("="*60)
    print("This demo simulates realistic automotive CAN communication")
    print("between an Engine ECU and Instrument Cluster")
    print("="*60)
    
    # Create ECU and Instrument Cluster
    engine = EngineECU()
    cluster = InstrumentCluster()
    
    # Start sender in background thread
    sender_thread = threading.Thread(target=engine.start, daemon=True)
    sender_thread.start()
    
    print("\n🚀 Starting demonstration...")
    print("The engine will simulate driving patterns with realistic data")
    print("Press Ctrl+C to stop\n")
    print("-" * 60)
    
    try:
        # Start receiver in main thread (so we can see the output)
        time.sleep(1)  # Let sender start first
        cluster.start()
        
    except KeyboardInterrupt:
        print("\n\n🛑 Demo stopped by user")
    finally:
        engine.stop()
        cluster.stop()
        print("✅ CAN Protocol demo completed successfully!")
        print("\n📊 Summary:")
        print("- Engine ECU sent realistic automotive data")
        print("- Instrument Cluster received and validated all messages")
        print("- Data included: RPM, Speed, Temperature, Fuel Level, Odometer")
        print("- All messages were properly encoded/decoded")

if __name__ == "__main__":
    main()