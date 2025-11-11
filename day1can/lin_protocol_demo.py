#!/usr/bin/env python3
"""
LIN Protocol Testing and Analysis Tool
Demonstrates LIN protocol testing for QA purposes
"""

import time
import threading
from collections import defaultdict
from datetime import datetime

class LINMessage:
    """LIN message representation"""
    def __init__(self, pid, data=None, timestamp=None):
        self.pid = pid  # Protected Identifier
        self.data = data or []
        self.timestamp = timestamp or time.time()
        self.checksum = self._calculate_checksum()
    
    def _calculate_checksum(self):
        """Calculate LIN checksum (simplified)"""
        checksum = self.pid
        for byte in self.data:
            checksum += byte
            if checksum > 255:
                checksum = (checksum & 0xFF) + 1
        return (~checksum) & 0xFF
    
    def validate_checksum(self):
        """Validate message checksum"""
        expected = self._calculate_checksum()
        return self.checksum == expected
    
    def __str__(self):
        return f"LIN[PID=0x{self.pid:02X}, Data={self.data}, CS=0x{self.checksum:02X}]"

class LINScheduler:
    """LIN Master scheduler implementation"""
    def __init__(self):
        self.schedule_table = []
        self.current_slot = 0
        self.running = False
        self.cycle_time = 0.1  # 100ms cycle
        self.message_log = []
        self.slave_responses = defaultdict(list)
    
    def add_schedule_entry(self, pid, slot_time, data_length=8, is_master_request=True):
        """Add entry to schedule table"""
        self.schedule_table.append({
            'pid': pid,
            'slot_time': slot_time,
            'data_length': data_length,
            'is_master_request': is_master_request,
            'last_execution': 0
        })
    
    def setup_window_control_schedule(self):
        """Setup typical window control LIN schedule"""
        # Master requests
        self.add_schedule_entry(0x20, 0.01, 2, True)   # Window position command
        self.add_schedule_entry(0x22, 0.02, 1, True)   # Mirror control command
        self.add_schedule_entry(0x24, 0.05, 3, True)   # Seat control command
        
        # Slave responses
        self.add_schedule_entry(0x21, 0.015, 4, False) # Window status response
        self.add_schedule_entry(0x23, 0.025, 2, False) # Mirror status response
        self.add_schedule_entry(0x25, 0.055, 5, False) # Seat status response
        
        # Diagnostic frames
        self.add_schedule_entry(0x3C, 1.0, 8, True)    # Master request frame
        self.add_schedule_entry(0x3D, 1.01, 8, False)  # Slave response frame
    
    def start_schedule(self, duration=None):
        """Start executing LIN schedule"""
        self.running = True
        start_time = time.time()
        
        print("Starting LIN schedule execution...")
        print("Schedule entries:", len(self.schedule_table))
        
        try:
            while self.running:
                current_time = time.time()
                
                if duration and (current_time - start_time) > duration:
                    break
                
                # Execute scheduled messages
                for entry in self.schedule_table:
                    time_since_last = current_time - entry['last_execution']
                    
                    if time_since_last >= entry['slot_time']:
                        self._execute_schedule_entry(entry, current_time)
                        entry['last_execution'] = current_time
                
                time.sleep(0.001)  # 1ms resolution
                
        except KeyboardInterrupt:
            print("Schedule stopped by user")
        finally:
            self.running = False
    
    def _execute_schedule_entry(self, entry, timestamp):
        """Execute a single schedule entry"""
        if entry['is_master_request']:
            # Generate master request
            data = self._generate_master_data(entry['pid'])
            message = LINMessage(entry['pid'], data, timestamp)
            self._send_message(message)
            
            # Simulate slave processing time
            time.sleep(0.002)  # 2ms processing delay
            
        else:
            # Simulate slave response
            response_data = self._simulate_slave_response(entry['pid'])
            message = LINMessage(entry['pid'], response_data, timestamp)
            self._receive_message(message)
    
    def _generate_master_data(self, pid):
        """Generate master request data based on PID"""
        data_patterns = {
            0x20: [0x80, 0x00],        # Window: 50% position
            0x22: [0x05],              # Mirror: fold command
            0x24: [0x10, 0x20, 0x30],  # Seat: position command
            0x3C: [0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08]  # Diagnostic
        }
        
        base_data = data_patterns.get(pid, [0x00])
        # Add some variation to simulate real operation
        timestamp_byte = int(time.time() * 100) % 256
        return base_data + [timestamp_byte]
    
    def _simulate_slave_response(self, pid):
        """Simulate slave response data"""
        response_patterns = {
            0x21: [0x80, 0x00, 0x01, 0x00],     # Window status: position + flags
            0x23: [0x05, 0x00],                 # Mirror status: position + error
            0x25: [0x10, 0x20, 0x30, 0x00, 0x01], # Seat status: positions + flags
            0x3D: [0x7F, 0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE]  # Diagnostic response
        }
        
        return response_patterns.get(pid, [0xFF])
    
    def _send_message(self, message):
        """Send LIN message (master)"""
        self.message_log.append(('TX', message))
        print(f"TX: {message}")
    
    def _receive_message(self, message):
        """Receive LIN message (slave response)"""
        self.message_log.append(('RX', message))
        self.slave_responses[message.pid].append(message)
        print(f"RX: {message}")
    
    def stop_schedule(self):
        """Stop schedule execution"""
        self.running = False
    
    def generate_timing_report(self):
        """Generate timing analysis report"""
        print("\n" + "="*60)
        print("LIN SCHEDULE TIMING REPORT")
        print("="*60)
        
        # Analyze message timing
        pid_timing = defaultdict(list)
        
        for i, (direction, message) in enumerate(self.message_log):
            if i > 0:
                prev_message = self.message_log[i-1][1]
                interval = message.timestamp - prev_message.timestamp
                pid_timing[message.pid].append(interval)
        
        print(f"Total Messages: {len(self.message_log)}")
        print(f"Unique PIDs: {len(pid_timing)}")
        
        print("\nPER-PID TIMING ANALYSIS:")
        print("-" * 40)
        
        for pid in sorted(pid_timing.keys()):
            intervals = pid_timing[pid]
            if intervals:
                avg_interval = sum(intervals) / len(intervals)
                min_interval = min(intervals)
                max_interval = max(intervals)
                jitter = max_interval - min_interval
                
                print(f"PID 0x{pid:02X}:")
                print(f"  Messages: {len(intervals) + 1}")
                print(f"  Avg Interval: {avg_interval:.4f}s")
                print(f"  Min Interval: {min_interval:.4f}s")
                print(f"  Max Interval: {max_interval:.4f}s")
                print(f"  Jitter: {jitter:.4f}s")
                print(f"  Expected Rate: {1/avg_interval:.2f} Hz")
        
        # Schedule compliance check
        print(f"\nSCHEDULE COMPLIANCE:")
        print("-" * 40)
        
        for entry in self.schedule_table:
            pid = entry['pid']
            expected_interval = entry['slot_time']
            
            if pid in pid_timing:
                actual_intervals = pid_timing[pid]
                avg_actual = sum(actual_intervals) / len(actual_intervals)
                tolerance = expected_interval * 0.05  # 5% tolerance
                
                compliance = abs(avg_actual - expected_interval) <= tolerance
                deviation = ((avg_actual - expected_interval) / expected_interval) * 100
                
                status = "PASS" if compliance else "FAIL"
                print(f"PID 0x{pid:02X}: {status} "
                      f"(Expected: {expected_interval:.4f}s, "
                      f"Actual: {avg_actual:.4f}s, "
                      f"Deviation: {deviation:.2f}%)")

class LINTester:
    """LIN protocol tester for QA validation"""
    
    def __init__(self):
        self.scheduler = LINScheduler()
        self.test_results = {}
    
    def test_window_control_scenario(self):
        """Test window control LIN communication"""
        print("Testing Window Control LIN Scenario")
        print("=" * 40)
        
        # Setup schedule
        self.scheduler.setup_window_control_schedule()
        
        # Run test
        test_duration = 5.0  # 5 seconds
        
        start_time = time.time()
        
        # Start scheduler in background
        scheduler_thread = threading.Thread(
            target=self.scheduler.start_schedule,
            args=(test_duration,)
        )
        scheduler_thread.start()
        
        # Wait for test completion
        scheduler_thread.join()
        
        end_time = time.time()
        actual_duration = end_time - start_time
        
        print(f"\nTest completed in {actual_duration:.2f} seconds")
        
        # Generate report
        self.scheduler.generate_timing_report()
        
        return self._evaluate_test_results()
    
    def test_diagnostic_communication(self):
        """Test LIN diagnostic communication"""
        print("\nTesting LIN Diagnostic Communication")
        print("=" * 40)
        
        # Simulate diagnostic request-response
        diagnostic_frames = [
            {'pid': 0x3C, 'data': [0x06, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]},  # Read DTC request
            {'pid': 0x3D, 'data': [0x46, 0x01, 0x23, 0x45, 0x00, 0x00, 0x00, 0x00]},  # DTC response
        ]
        
        for frame in diagnostic_frames:
            message = LINMessage(frame['pid'], frame['data'])
            print(f"Diagnostic: {message}")
            
            # Validate checksum
            if message.validate_checksum():
                print(f"  Checksum: VALID")
            else:
                print(f"  Checksum: INVALID")
        
        return True
    
    def test_error_conditions(self):
        """Test LIN error handling"""
        print("\nTesting LIN Error Conditions")
        print("=" * 40)
        
        # Test checksum errors
        message = LINMessage(0x21, [0x01, 0x02, 0x03, 0x04])
        message.checksum = 0x00  # Intentionally wrong checksum
        
        print(f"Error Test Message: {message}")
        print(f"Checksum Valid: {message.validate_checksum()}")
        
        # Test schedule timing violations
        print("\nTiming Violation Simulation:")
        violation_detected = self._simulate_timing_violation()
        print(f"Timing Violation Detected: {violation_detected}")
        
        return True
    
    def _simulate_timing_violation(self):
        """Simulate and detect timing violations"""
        # Simulate messages arriving outside expected schedule
        expected_interval = 0.01  # 10ms
        actual_intervals = [0.008, 0.012, 0.015, 0.009, 0.020]  # Some outside tolerance
        
        tolerance = expected_interval * 0.1  # 10% tolerance
        violations = []
        
        for i, interval in enumerate(actual_intervals):
            if abs(interval - expected_interval) > tolerance:
                violations.append(f"Message {i}: {interval:.3f}s (expected: {expected_interval:.3f}s)")
        
        if violations:
            print("  Violations found:")
            for violation in violations:
                print(f"    {violation}")
            return True
        else:
            print("  No violations detected")
            return False
    
    def _evaluate_test_results(self):
        """Evaluate overall test results"""
        print(f"\nTEST EVALUATION:")
        print("-" * 40)
        
        # Check message completeness
        total_messages = len(self.scheduler.message_log)
        expected_messages = len(self.scheduler.schedule_table) * 5  # Rough estimate
        
        completeness = (total_messages / expected_messages) * 100 if expected_messages > 0 else 0
        print(f"Message Completeness: {completeness:.1f}%")
        
        # Check response rates
        master_requests = sum(1 for direction, _ in self.scheduler.message_log if direction == 'TX')
        slave_responses = sum(1 for direction, _ in self.scheduler.message_log if direction == 'RX')
        
        response_rate = (slave_responses / master_requests) * 100 if master_requests > 0 else 0
        print(f"Slave Response Rate: {response_rate:.1f}%")
        
        # Overall result
        test_passed = completeness > 80 and response_rate > 90
        result = "PASS" if test_passed else "FAIL"
        print(f"Overall Test Result: {result}")
        
        return test_passed

def demonstrate_lin_testing():
    """Main LIN demonstration function"""
    print("LIN Protocol QA Testing Demonstration")
    print("=" * 50)
    
    tester = LINTester()
    
    try:
        # Run test scenarios
        print("\n1. Window Control Communication Test")
        result1 = tester.test_window_control_scenario()
        
        print("\n2. Diagnostic Communication Test")
        result2 = tester.test_diagnostic_communication()
        
        print("\n3. Error Condition Tests")
        result3 = tester.test_error_conditions()
        
        # Summary
        print("\n" + "="*50)
        print("FINAL TEST SUMMARY")
        print("="*50)
        print(f"Window Control Test: {'PASS' if result1 else 'FAIL'}")
        print(f"Diagnostic Test: {'PASS' if result2 else 'FAIL'}")
        print(f"Error Handling Test: {'PASS' if result3 else 'FAIL'}")
        
        overall_result = all([result1, result2, result3])
        print(f"Overall Result: {'PASS' if overall_result else 'FAIL'}")
        
    except Exception as e:
        print(f"Demo failed: {e}")

if __name__ == "__main__":
    demonstrate_lin_testing()