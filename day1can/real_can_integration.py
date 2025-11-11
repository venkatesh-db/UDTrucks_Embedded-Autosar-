#!/usr/bin/env python3
"""
Real CAN Hardware Integration Script
Connects our demonstrations to actual CAN hardware using python-can library
"""

import time
import json
from datetime import datetime
from collections import defaultdict

# Try to import python-can (install with: pip install python-can)
try:
    import can
    CAN_AVAILABLE = True
except ImportError:
    CAN_AVAILABLE = False
    print("python-can not installed. Running in simulation mode.")
    print("Install with: pip install python-can[pcan]")

# Import our demonstration modules
from can_analyzer_demo_simulation import CANAnalyzer, MockCANMessage

class RealCANInterface:
    """Interface to real CAN hardware"""
    
    def __init__(self, interface_type='pcan', channel='PCAN_USBBUS1', bitrate=500000):
        self.interface_type = interface_type
        self.channel = channel
        self.bitrate = bitrate
        self.bus = None
        self.analyzer = CANAnalyzer()
        self.message_log = []
        
    def connect(self):
        """Connect to CAN hardware"""
        if not CAN_AVAILABLE:
            print("Running in simulation mode - no real CAN hardware")
            return True
            
        try:
            self.bus = can.interface.Bus(
                bustype=self.interface_type,
                channel=self.channel,
                bitrate=self.bitrate
            )
            print(f"Connected to {self.interface_type} on {self.channel} at {self.bitrate} bps")
            return True
        except Exception as e:
            print(f"Failed to connect to CAN hardware: {e}")
            return False
    
    def send_demo_messages(self):
        """Send our demonstration messages to real CAN bus"""
        test_messages = [
            {'id': 0x123, 'data': [0x01, 0x02, 0x03, 0x04]},
            {'id': 0x456, 'data': [0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80]},
            {'id': 0x789, 'data': [0xFF, 0x00, 0xAA, 0x55]},
            {'id': 0x100, 'data': [0x80, 0x40, 0x20, 0x10]},  # High priority
            {'id': 0x7FF, 'data': [0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF]},  # Low priority
        ]
        
        print("\n🚀 Sending demonstration messages to CAN bus...")
        print("="*60)
        
        for i, msg_data in enumerate(test_messages):
            timestamp = time.time()
            
            if CAN_AVAILABLE and self.bus:
                # Send to real CAN bus
                msg = can.Message(
                    arbitration_id=msg_data['id'],
                    data=msg_data['data'],
                    is_extended_id=False
                )
                
                try:
                    self.bus.send(msg)
                    success = True
                except Exception as e:
                    print(f"Send error: {e}")
                    success = False
            else:
                # Simulation mode
                success = True
            
            # Log message for analysis
            log_entry = {
                'timestamp': timestamp,
                'id': msg_data['id'],
                'data': msg_data['data'],
                'success': success,
                'sequence': i + 1
            }
            self.message_log.append(log_entry)
            
            # Display sent message
            data_str = ' '.join([f'{b:02X}' for b in msg_data['data']])
            status = "✅ SENT" if success else "❌ FAILED"
            print(f"{status} | ID: 0x{msg_data['id']:03X} | Data: [{data_str}] | Length: {len(msg_data['data'])}")
            
            time.sleep(0.1)  # 100ms between messages
    
    def monitor_can_bus(self, duration=10):
        """Monitor CAN bus and analyze received messages"""
        if not CAN_AVAILABLE or not self.bus:
            print("CAN monitoring not available - using simulation")
            self._simulate_monitoring(duration)
            return
        
        print(f"\n📡 Monitoring CAN bus for {duration} seconds...")
        print("="*60)
        
        start_time = time.time()
        message_count = 0
        
        while (time.time() - start_time) < duration:
            try:
                # Receive message with timeout
                msg = self.bus.recv(timeout=1.0)
                
                if msg:
                    message_count += 1
                    timestamp = time.time()
                    
                    # Log received message
                    log_entry = {
                        'timestamp': timestamp,
                        'id': msg.arbitration_id,
                        'data': list(msg.data),
                        'direction': 'RX'
                    }
                    self.message_log.append(log_entry)
                    
                    # Display received message
                    data_str = ' '.join([f'{b:02X}' for b in msg.data])
                    print(f"📥 RX | ID: 0x{msg.arbitration_id:03X} | Data: [{data_str}] | DLC: {len(msg.data)}")
                    
            except can.CanOperationError:
                continue
            except KeyboardInterrupt:
                print("\nMonitoring stopped by user")
                break
        
        print(f"\nReceived {message_count} messages in {duration} seconds")
        return message_count
    
    def _simulate_monitoring(self, duration):
        """Simulate CAN monitoring for demo purposes"""
        import random
        
        print("📡 Simulated CAN monitoring active...")
        
        # Simulate some common automotive messages
        automotive_messages = [
            {'id': 0x0C9, 'name': 'Engine RPM', 'data_gen': lambda: [random.randint(0, 255) for _ in range(8)]},
            {'id': 0x0AA, 'name': 'Vehicle Speed', 'data_gen': lambda: [random.randint(0, 200), 0, 0, 0]},
            {'id': 0x128, 'name': 'Steering Angle', 'data_gen': lambda: [random.randint(0, 255) for _ in range(4)]},
            {'id': 0x318, 'name': 'Gear Position', 'data_gen': lambda: [random.randint(1, 8), 0, 0, 0]},
        ]
        
        start_time = time.time()
        message_count = 0
        
        while (time.time() - start_time) < duration:
            # Random message selection
            msg_template = random.choice(automotive_messages)
            data = msg_template['data_gen']()
            
            log_entry = {
                'timestamp': time.time(),
                'id': msg_template['id'],
                'data': data,
                'name': msg_template['name'],
                'direction': 'RX'
            }
            self.message_log.append(log_entry)
            
            data_str = ' '.join([f'{b:02X}' for b in data])
            print(f"📥 RX | {msg_template['name']} | ID: 0x{msg_template['id']:03X} | Data: [{data_str}]")
            
            message_count += 1
            time.sleep(0.2 + random.uniform(0, 0.3))  # Variable timing
        
        print(f"\nSimulated {message_count} messages in {duration} seconds")
    
    def analyze_traffic(self):
        """Analyze captured CAN traffic using our demo analyzer"""
        if not self.message_log:
            print("No messages to analyze")
            return
        
        print("\n📊 CAN Traffic Analysis")
        print("="*60)
        
        # Basic statistics
        total_messages = len(self.message_log)
        unique_ids = len(set(msg['id'] for msg in self.message_log))
        time_span = self.message_log[-1]['timestamp'] - self.message_log[0]['timestamp']
        message_rate = total_messages / time_span if time_span > 0 else 0
        
        print(f"Total Messages: {total_messages}")
        print(f"Unique Message IDs: {unique_ids}")
        print(f"Time Span: {time_span:.2f} seconds")
        print(f"Message Rate: {message_rate:.1f} messages/second")
        
        # Message frequency analysis
        id_counts = defaultdict(int)
        id_timing = defaultdict(list)
        
        last_timestamp = {}
        for msg in self.message_log:
            msg_id = msg['id']
            timestamp = msg['timestamp']
            
            id_counts[msg_id] += 1
            
            if msg_id in last_timestamp:
                interval = timestamp - last_timestamp[msg_id]
                id_timing[msg_id].append(interval)
            
            last_timestamp[msg_id] = timestamp
        
        # Display frequency analysis
        print("\n📈 Message Frequency Analysis:")
        print("-" * 40)
        sorted_ids = sorted(id_counts.items(), key=lambda x: x[1], reverse=True)
        
        for msg_id, count in sorted_ids[:10]:  # Top 10
            frequency = count / time_span if time_span > 0 else 0
            
            if msg_id in id_timing and id_timing[msg_id]:
                avg_interval = sum(id_timing[msg_id]) / len(id_timing[msg_id])
                cycle_time = avg_interval * 1000  # Convert to ms
            else:
                cycle_time = 0
            
            print(f"ID 0x{msg_id:03X}: {count:3d} msgs, {frequency:5.1f} Hz, {cycle_time:6.1f}ms cycle")
        
        # Bus load calculation (simplified)
        total_bits = sum(64 + (len(msg.get('data', [])) * 8) for msg in self.message_log)
        bus_load = (total_bits / (500000 * time_span)) * 100 if time_span > 0 else 0
        
        print(f"\n🚌 Estimated Bus Load: {bus_load:.2f}%")
        
        # Error analysis
        errors = [msg for msg in self.message_log if not msg.get('success', True)]
        if errors:
            print(f"⚠️  Transmission Errors: {len(errors)}")
        else:
            print("✅ No transmission errors detected")
    
    def export_to_vector_asc(self, filename=None):
        """Export captured data to Vector ASC format for CANalyzer"""
        if not filename:
            filename = f"can_capture_{datetime.now().strftime('%Y%m%d_%H%M%S')}.asc"
        
        print(f"\n💾 Exporting to Vector ASC format: {filename}")
        
        with open(filename, 'w') as f:
            # ASC file header
            f.write("date Mon Jan 01 12:00:00.000 2024\n")
            f.write("base hex  timestamps absolute\n")
            f.write("internal events logged\n")
            f.write("// version 13.0.0\n")
            f.write("Begin Triggerblock Mon Jan 01 12:00:00.000 2024\n")
            
            # Convert messages to ASC format
            start_time = self.message_log[0]['timestamp'] if self.message_log else time.time()
            
            for msg in self.message_log:
                # Calculate relative timestamp in seconds
                rel_time = msg['timestamp'] - start_time
                
                # ASC format: timestamp   1  123x             Rx   d 4 01 02 03 04
                data_str = ' '.join([f'{b:02x}' for b in msg.get('data', [])])
                direction = msg.get('direction', 'Tx')
                
                f.write(f"{rel_time:10.6f}   1  {msg['id']:3x}x             {direction}   d {len(msg.get('data', []))}")
                if data_str:
                    f.write(f" {data_str}")
                f.write("\n")
            
            f.write("End TriggerBlock\n")
        
        print(f"✅ Exported {len(self.message_log)} messages to {filename}")
        print("   → Import this file into Vector CANalyzer for professional analysis")
        
        return filename
    
    def disconnect(self):
        """Disconnect from CAN hardware"""
        if self.bus:
            self.bus.shutdown()
            print("Disconnected from CAN hardware")

def demonstrate_real_can_integration():
    """Main demonstration of real CAN integration"""
    print("🚗 Real CAN Hardware Integration Demonstration")
    print("=" * 60)
    
    # Initialize CAN interface
    can_interface = RealCANInterface()
    
    try:
        # Connect to hardware
        if can_interface.connect():
            
            # Send demonstration messages
            can_interface.send_demo_messages()
            
            # Monitor for incoming traffic
            print(f"\nPress Ctrl+C to stop monitoring early...")
            can_interface.monitor_can_bus(duration=5)
            
            # Analyze captured traffic
            can_interface.analyze_traffic()
            
            # Export to professional format
            asc_file = can_interface.export_to_vector_asc()
            
            print(f"\n🎯 Integration Complete!")
            print(f"   - Messages sent and monitored")
            print(f"   - Traffic analysis performed") 
            print(f"   - Data exported to: {asc_file}")
            print(f"   - Ready for professional tool analysis")
            
    except KeyboardInterrupt:
        print("\nStopped by user")
    except Exception as e:
        print(f"Error during demonstration: {e}")
    finally:
        can_interface.disconnect()

if __name__ == "__main__":
    demonstrate_real_can_integration()