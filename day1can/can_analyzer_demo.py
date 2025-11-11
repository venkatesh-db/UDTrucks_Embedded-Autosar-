#!/usr/bin/env python3
"""
CAN Bus Analysis and Testing Tool
Demonstrates CAN protocol testing for QA purposes
"""

import can
import time
import threading
from collections import defaultdict, deque
from datetime import datetime
import statistics

class CANAnalyzer:
    def __init__(self, channel='vcan0', bustype='socketcan'):
        """Initialize CAN analyzer"""
        try:
            self.bus = can.interface.Bus(channel=channel, bustype=bustype)
            self.running = False
            self.message_stats = defaultdict(lambda: {
                'count': 0,
                'intervals': deque(maxlen=100),
                'last_timestamp': None,
                'data_variations': set(),
                'errors': 0
            })
            self.total_errors = 0
            self.start_time = None
        except Exception as e:
            print(f"Failed to initialize CAN bus: {e}")
            self.bus = None
    
    def start_monitoring(self, duration=None):
        """Start monitoring CAN bus traffic"""
        if not self.bus:
            return
        
        self.running = True
        self.start_time = time.time()
        
        print(f"Starting CAN monitoring on {self.bus.channel_info}")
        print("Press Ctrl+C to stop monitoring")
        
        try:
            end_time = time.time() + duration if duration else None
            
            for message in self.bus:
                if not self.running:
                    break
                
                if end_time and time.time() > end_time:
                    break
                
                self._process_message(message)
                
        except KeyboardInterrupt:
            print("\nMonitoring stopped by user")
        finally:
            self.running = False
    
    def _process_message(self, message):
        """Process received CAN message"""
        if message.is_error_frame:
            self.total_errors += 1
            return
        
        msg_id = message.arbitration_id
        stats = self.message_stats[msg_id]
        
        # Update message count
        stats['count'] += 1
        
        # Calculate interval
        if stats['last_timestamp']:
            interval = message.timestamp - stats['last_timestamp']
            stats['intervals'].append(interval)
        
        stats['last_timestamp'] = message.timestamp
        
        # Track data variations
        data_tuple = tuple(message.data)
        stats['data_variations'].add(data_tuple)
        
        # Real-time display for active monitoring
        if stats['count'] % 10 == 0:  # Display every 10th message
            self._display_realtime_stats(msg_id, stats)
    
    def _display_realtime_stats(self, msg_id, stats):
        """Display real-time statistics"""
        avg_interval = statistics.mean(stats['intervals']) if stats['intervals'] else 0
        print(f"ID 0x{msg_id:03X}: {stats['count']} msgs, "
              f"avg interval: {avg_interval:.3f}s, "
              f"variations: {len(stats['data_variations'])}")
    
    def send_test_messages(self, test_duration=10):
        """Send test messages for demonstration"""
        if not self.bus:
            return
        
        print(f"Sending test messages for {test_duration} seconds...")
        
        test_messages = [
            {'id': 0x123, 'data': [0x01, 0x02, 0x03, 0x04], 'interval': 0.1},
            {'id': 0x456, 'data': [0x05, 0x06, 0x07, 0x08], 'interval': 0.2},
            {'id': 0x789, 'data': [0x09, 0x0A, 0x0B, 0x0C], 'interval': 0.05}
        ]
        
        def send_periodic_message(msg_config):
            start_time = time.time()
            counter = 0
            
            while time.time() - start_time < test_duration:
                # Modify data to simulate changing values
                data = msg_config['data'].copy()
                data[-1] = counter % 256
                
                message = can.Message(
                    arbitration_id=msg_config['id'],
                    data=data,
                    is_extended_id=False
                )
                
                try:
                    self.bus.send(message)
                    counter += 1
                    time.sleep(msg_config['interval'])
                except can.CanError as e:
                    print(f"Failed to send message: {e}")
        
        # Start sending threads
        threads = []
        for msg_config in test_messages:
            thread = threading.Thread(target=send_periodic_message, args=(msg_config,))
            thread.start()
            threads.append(thread)
        
        # Wait for all threads to complete
        for thread in threads:
            thread.join()
        
        print("Test message transmission completed")
    
    def test_arbitration(self):
        """Demonstrate CAN arbitration behavior"""
        if not self.bus:
            return
        
        print("Testing CAN arbitration...")
        
        # Create messages with different priorities
        high_priority = can.Message(arbitration_id=0x100, data=[0x01])
        medium_priority = can.Message(arbitration_id=0x200, data=[0x02])
        low_priority = can.Message(arbitration_id=0x300, data=[0x03])
        
        messages = [high_priority, medium_priority, low_priority]
        
        # Send messages simultaneously (simulated)
        send_times = []
        for msg in messages:
            start_time = time.time()
            try:
                self.bus.send(msg)
                send_time = time.time() - start_time
                send_times.append((msg.arbitration_id, send_time))
                print(f"Sent ID 0x{msg.arbitration_id:03X} in {send_time:.6f}s")
            except can.CanError as e:
                print(f"Failed to send message 0x{msg.arbitration_id:03X}: {e}")
        
        return send_times
    
    def generate_report(self):
        """Generate comprehensive analysis report"""
        if not self.message_stats:
            print("No data to report")
            return
        
        total_duration = time.time() - self.start_time if self.start_time else 0
        
        print("\n" + "="*60)
        print("CAN BUS ANALYSIS REPORT")
        print("="*60)
        print(f"Analysis Duration: {total_duration:.2f} seconds")
        print(f"Total Error Frames: {self.total_errors}")
        print(f"Unique Message IDs: {len(self.message_stats)}")
        
        # Calculate bus utilization
        total_messages = sum(stats['count'] for stats in self.message_stats.values())
        if total_duration > 0:
            msg_rate = total_messages / total_duration
            print(f"Message Rate: {msg_rate:.2f} messages/second")
        
        print("\nPER-MESSAGE STATISTICS:")
        print("-" * 60)
        
        for msg_id in sorted(self.message_stats.keys()):
            stats = self.message_stats[msg_id]
            
            print(f"\nMessage ID: 0x{msg_id:03X}")
            print(f"  Total Messages: {stats['count']}")
            
            if stats['intervals']:
                intervals = list(stats['intervals'])
                avg_interval = statistics.mean(intervals)
                min_interval = min(intervals)
                max_interval = max(intervals)
                jitter = max_interval - min_interval
                
                print(f"  Average Interval: {avg_interval:.4f}s")
                print(f"  Min Interval: {min_interval:.4f}s")
                print(f"  Max Interval: {max_interval:.4f}s")
                print(f"  Jitter: {jitter:.4f}s")
                print(f"  Frequency: {1/avg_interval:.2f} Hz")
            
            print(f"  Data Variations: {len(stats['data_variations'])}")
            
            # Show some data examples
            if stats['data_variations']:
                examples = list(stats['data_variations'])[:3]
                print(f"  Data Examples: {examples}")
        
        # Quality metrics
        print(f"\nQUALITY METRICS:")
        print("-" * 60)
        
        error_rate = self.total_errors / total_messages if total_messages > 0 else 0
        print(f"Error Rate: {error_rate:.6f} ({error_rate*100:.4f}%)")
        
        # Timing analysis
        all_intervals = []
        for stats in self.message_stats.values():
            all_intervals.extend(stats['intervals'])
        
        if all_intervals:
            overall_jitter = max(all_intervals) - min(all_intervals)
            print(f"Overall Timing Jitter: {overall_jitter:.4f}s")

def demonstrate_can_testing():
    """Main demonstration function"""
    print("CAN Protocol QA Testing Demonstration")
    print("=====================================")
    
    # Create analyzer
    analyzer = CANAnalyzer()
    
    if not analyzer.bus:
        print("Cannot proceed without CAN interface")
        print("To run this demo:")
        print("1. Install python-can: pip install python-can")
        print("2. Create virtual CAN interface:")
        print("   sudo modprobe vcan")
        print("   sudo ip link add dev vcan0 type vcan")
        print("   sudo ip link set up vcan0")
        return
    
    try:
        # Demonstrate different test scenarios
        print("\n1. Testing CAN arbitration...")
        analyzer.test_arbitration()
        
        print("\n2. Sending test traffic...")
        # Start monitoring in background
        monitor_thread = threading.Thread(
            target=analyzer.start_monitoring, 
            args=(15,)  # Monitor for 15 seconds
        )
        monitor_thread.start()
        
        # Give monitoring a moment to start
        time.sleep(1)
        
        # Send test messages
        analyzer.send_test_messages(test_duration=10)
        
        # Wait for monitoring to complete
        monitor_thread.join()
        
        print("\n3. Generating analysis report...")
        analyzer.generate_report()
        
    except Exception as e:
        print(f"Demo failed: {e}")
    
    finally:
        analyzer.running = False

if __name__ == "__main__":
    demonstrate_can_testing()