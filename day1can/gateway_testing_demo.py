#!/usr/bin/env python3
"""
Multi-Protocol Gateway Testing Tool
Demonstrates cross-protocol communication testing for automotive QA
"""

import time
import threading
import json
from collections import defaultdict, deque
from datetime import datetime
import queue

class Message:
    """Generic message class for all protocols"""
    def __init__(self, protocol, msg_id, data, timestamp=None):
        self.protocol = protocol
        self.msg_id = msg_id
        self.data = data
        self.timestamp = timestamp or time.time()
        self.size = len(data) if isinstance(data, (list, bytes)) else 0
    
    def __str__(self):
        return f"{self.protocol}[ID=0x{self.msg_id:03X}, Data={self.data}, Size={self.size}]"

class ProtocolGateway:
    """Simulates automotive protocol gateway functionality"""
    
    def __init__(self):
        self.routing_table = {}
        self.protocol_handlers = {
            'CAN': self._handle_can_message,
            'LIN': self._handle_lin_message,
            'FlexRay': self._handle_flexray_message,
            'Ethernet': self._handle_ethernet_message
        }
        self.message_queues = {
            'CAN': queue.Queue(),
            'LIN': queue.Queue(),
            'FlexRay': queue.Queue(),
            'Ethernet': queue.Queue()
        }
        self.statistics = defaultdict(lambda: {
            'rx_count': 0,
            'tx_count': 0,
            'error_count': 0,
            'routing_failures': 0
        })
        self.running = False
        self.translation_rules = {}
    
    def setup_routing_rules(self):
        """Setup message routing rules between protocols"""
        # CAN to Ethernet routing (typical for diagnostics)
        self.add_routing_rule('CAN', 0x7DF, 'Ethernet', 0x18DA00F1)  # OBD-II request
        self.add_routing_rule('CAN', 0x7E8, 'Ethernet', 0x18DAF110)  # OBD-II response
        
        # CAN to LIN routing (body control)
        self.add_routing_rule('CAN', 0x123, 'LIN', 0x20)  # Window control
        self.add_routing_rule('CAN', 0x124, 'LIN', 0x22)  # Mirror control
        
        # FlexRay to CAN routing (safety to comfort)
        self.add_routing_rule('FlexRay', 0x100, 'CAN', 0x456)  # Safety status
        
        # Ethernet to CAN routing (infotainment to vehicle)
        self.add_routing_rule('Ethernet', 0x1000, 'CAN', 0x789)  # Media control
        
        print(f"Configured {len(self.routing_table)} routing rules")
    
    def add_routing_rule(self, src_protocol, src_id, dst_protocol, dst_id):
        """Add message routing rule"""
        rule_key = (src_protocol, src_id)
        self.routing_table[rule_key] = (dst_protocol, dst_id)
        
        # Add translation rule if needed
        if src_protocol != dst_protocol:
            self.translation_rules[rule_key] = self._create_translation_rule(
                src_protocol, dst_protocol, src_id, dst_id
            )
    
    def _create_translation_rule(self, src_protocol, dst_protocol, src_id, dst_id):
        """Create protocol translation rule"""
        return {
            'src_protocol': src_protocol,
            'dst_protocol': dst_protocol,
            'src_id': src_id,
            'dst_id': dst_id,
            'data_transform': self._get_data_transform_function(src_protocol, dst_protocol)
        }
    
    def _get_data_transform_function(self, src_protocol, dst_protocol):
        """Get data transformation function between protocols"""
        transform_key = f"{src_protocol}_to_{dst_protocol}"
        
        transforms = {
            'CAN_to_Ethernet': self._can_to_ethernet_transform,
            'CAN_to_LIN': self._can_to_lin_transform,
            'LIN_to_CAN': self._lin_to_can_transform,
            'FlexRay_to_CAN': self._flexray_to_can_transform,
            'Ethernet_to_CAN': self._ethernet_to_can_transform
        }
        
        return transforms.get(transform_key, self._default_transform)
    
    def _can_to_ethernet_transform(self, can_data):
        """Transform CAN data to Ethernet format"""
        # Add Ethernet header and padding
        eth_header = [0x00, 0x01, 0x02, 0x03]  # Simplified header
        return eth_header + can_data + [0x00] * (64 - len(can_data) - len(eth_header))
    
    def _can_to_lin_transform(self, can_data):
        """Transform CAN data to LIN format"""
        # LIN typically uses fewer bytes, extract relevant data
        return can_data[:2] if len(can_data) >= 2 else can_data
    
    def _lin_to_can_transform(self, lin_data):
        """Transform LIN data to CAN format"""
        # Pad LIN data to CAN format
        return lin_data + [0x00] * (8 - len(lin_data))
    
    def _flexray_to_can_transform(self, flexray_data):
        """Transform FlexRay data to CAN format"""
        # Extract first 8 bytes for CAN
        return flexray_data[:8] if len(flexray_data) >= 8 else flexray_data
    
    def _ethernet_to_can_transform(self, eth_data):
        """Transform Ethernet data to CAN format"""
        # Extract payload from Ethernet frame
        if len(eth_data) > 4:  # Skip header
            payload = eth_data[4:12]  # Extract 8 bytes
            return payload
        return eth_data[:8]
    
    def _default_transform(self, data):
        """Default data transformation (no change)"""
        return data
    
    def start_gateway(self):
        """Start gateway processing"""
        self.running = True
        
        # Start processing threads for each protocol
        threads = []
        for protocol in self.protocol_handlers:
            thread = threading.Thread(
                target=self._process_protocol_queue,
                args=(protocol,)
            )
            thread.daemon = True
            thread.start()
            threads.append(thread)
        
        print("Gateway started, processing messages...")
        return threads
    
    def stop_gateway(self):
        """Stop gateway processing"""
        self.running = False
        print("Gateway stopped")
    
    def _process_protocol_queue(self, protocol):
        """Process messages for a specific protocol"""
        msg_queue = self.message_queues[protocol]
        
        while self.running:
            try:
                message = msg_queue.get(timeout=0.1)
                self._route_message(message)
                msg_queue.task_done()
            except queue.Empty:
                continue
            except Exception as e:
                self.statistics[protocol]['error_count'] += 1
                print(f"Error processing {protocol} message: {e}")
    
    def _route_message(self, message):
        """Route message according to routing table"""
        route_key = (message.protocol, message.msg_id)
        
        if route_key in self.routing_table:
            dst_protocol, dst_id = self.routing_table[route_key]
            
            # Apply translation if needed
            translated_data = message.data
            if route_key in self.translation_rules:
                rule = self.translation_rules[route_key]
                translated_data = rule['data_transform'](message.data)
            
            # Create routed message
            routed_message = Message(
                protocol=dst_protocol,
                msg_id=dst_id,
                data=translated_data,
                timestamp=time.time()
            )
            
            # Send to destination protocol
            self._send_to_protocol(routed_message)
            
            # Update statistics
            self.statistics[message.protocol]['rx_count'] += 1
            self.statistics[dst_protocol]['tx_count'] += 1
            
            print(f"Routed: {message} -> {routed_message}")
            
        else:
            # No routing rule found
            self.statistics[message.protocol]['routing_failures'] += 1
            print(f"No routing rule for: {message}")
    
    def _send_to_protocol(self, message):
        """Send message to destination protocol"""
        if message.protocol in self.protocol_handlers:
            handler = self.protocol_handlers[message.protocol]
            handler(message)
    
    def _handle_can_message(self, message):
        """Handle outbound CAN message"""
        print(f"CAN TX: {message}")
    
    def _handle_lin_message(self, message):
        """Handle outbound LIN message"""
        print(f"LIN TX: {message}")
    
    def _handle_flexray_message(self, message):
        """Handle outbound FlexRay message"""
        print(f"FlexRay TX: {message}")
    
    def _handle_ethernet_message(self, message):
        """Handle outbound Ethernet message"""
        print(f"Ethernet TX: {message}")
    
    def inject_message(self, protocol, msg_id, data):
        """Inject message into gateway for testing"""
        message = Message(protocol, msg_id, data)
        
        if protocol in self.message_queues:
            self.message_queues[protocol].put(message)
            print(f"Injected: {message}")
        else:
            print(f"Unknown protocol: {protocol}")
    
    def generate_statistics_report(self):
        """Generate gateway statistics report"""
        print("\n" + "="*60)
        print("GATEWAY STATISTICS REPORT")
        print("="*60)
        
        total_rx = sum(stats['rx_count'] for stats in self.statistics.values())
        total_tx = sum(stats['tx_count'] for stats in self.statistics.values())
        total_errors = sum(stats['error_count'] for stats in self.statistics.values())
        total_routing_failures = sum(stats['routing_failures'] for stats in self.statistics.values())
        
        print(f"Total Messages Received: {total_rx}")
        print(f"Total Messages Transmitted: {total_tx}")
        print(f"Total Errors: {total_errors}")
        print(f"Routing Failures: {total_routing_failures}")
        
        if total_rx > 0:
            success_rate = ((total_rx - total_routing_failures) / total_rx) * 100
            print(f"Routing Success Rate: {success_rate:.2f}%")
        
        print(f"\nPER-PROTOCOL STATISTICS:")
        print("-" * 40)
        
        for protocol, stats in self.statistics.items():
            if any(stats.values()):  # Only show protocols with activity
                print(f"{protocol}:")
                print(f"  RX: {stats['rx_count']}")
                print(f"  TX: {stats['tx_count']}")
                print(f"  Errors: {stats['error_count']}")
                print(f"  Routing Failures: {stats['routing_failures']}")

class GatewayTester:
    """Test suite for protocol gateway functionality"""
    
    def __init__(self):
        self.gateway = ProtocolGateway()
        self.test_results = {}
    
    def setup_test_environment(self):
        """Setup test environment"""
        print("Setting up gateway test environment...")
        self.gateway.setup_routing_rules()
        threads = self.gateway.start_gateway()
        time.sleep(0.1)  # Allow gateway to start
        return threads
    
    def test_can_to_ethernet_routing(self):
        """Test CAN to Ethernet message routing"""
        print("\nTesting CAN to Ethernet routing...")
        
        # Test OBD-II diagnostic request
        test_cases = [
            {'id': 0x7DF, 'data': [0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00]},  # Get PIDs
            {'id': 0x7DF, 'data': [0x02, 0x01, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00]},  # Get coolant temp
        ]
        
        for case in test_cases:
            self.gateway.inject_message('CAN', case['id'], case['data'])
            time.sleep(0.1)  # Allow processing
        
        return True
    
    def test_can_to_lin_routing(self):
        """Test CAN to LIN message routing"""
        print("\nTesting CAN to LIN routing...")
        
        # Test body control messages
        test_cases = [
            {'id': 0x123, 'data': [0x80, 0x00, 0x01, 0x02]},  # Window control
            {'id': 0x124, 'data': [0x05, 0x03]},              # Mirror control
        ]
        
        for case in test_cases:
            self.gateway.inject_message('CAN', case['id'], case['data'])
            time.sleep(0.1)
        
        return True
    
    def test_flexray_to_can_routing(self):
        """Test FlexRay to CAN message routing"""
        print("\nTesting FlexRay to CAN routing...")
        
        # Test safety system messages
        flexray_data = [0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A]
        self.gateway.inject_message('FlexRay', 0x100, flexray_data)
        time.sleep(0.1)
        
        return True
    
    def test_ethernet_to_can_routing(self):
        """Test Ethernet to CAN message routing"""
        print("\nTesting Ethernet to CAN routing...")
        
        # Test infotainment to vehicle communication
        eth_frame = [0x00, 0x01, 0x02, 0x03, 0x10, 0x20, 0x30, 0x40] + [0x00] * 56
        self.gateway.inject_message('Ethernet', 0x1000, eth_frame)
        time.sleep(0.1)
        
        return True
    
    def test_routing_failures(self):
        """Test handling of messages with no routing rules"""
        print("\nTesting routing failure handling...")
        
        # Send messages with no routing rules
        self.gateway.inject_message('CAN', 0x999, [0x01, 0x02, 0x03])
        self.gateway.inject_message('LIN', 0x99, [0x04, 0x05])
        time.sleep(0.1)
        
        return True
    
    def test_data_transformation(self):
        """Test protocol data transformation"""
        print("\nTesting data transformation...")
        
        # Test different transformation scenarios
        gateway = self.gateway
        
        # CAN to Ethernet transformation
        can_data = [0x01, 0x02, 0x03, 0x04]
        eth_result = gateway._can_to_ethernet_transform(can_data)
        print(f"CAN to Ethernet: {can_data} -> {eth_result[:8]}...")  # Show first 8 bytes
        
        # CAN to LIN transformation
        lin_result = gateway._can_to_lin_transform(can_data)
        print(f"CAN to LIN: {can_data} -> {lin_result}")
        
        return True
    
    def run_performance_test(self, duration=5.0, message_rate=10):
        """Run gateway performance test"""
        print(f"\nRunning performance test ({duration}s, {message_rate} msg/s)...")
        
        start_time = time.time()
        message_count = 0
        
        while time.time() - start_time < duration:
            # Inject test messages at specified rate
            test_messages = [
                ('CAN', 0x7DF, [0x02, 0x01, 0x00]),
                ('CAN', 0x123, [0x80, 0x00]),
                ('FlexRay', 0x100, [0x01, 0x02, 0x03]),
                ('Ethernet', 0x1000, [0x00] * 64)
            ]
            
            for protocol, msg_id, data in test_messages:
                self.gateway.inject_message(protocol, msg_id, data)
                message_count += 1
            
            time.sleep(1.0 / message_rate)
        
        print(f"Performance test completed: {message_count} messages in {duration}s")
        return message_count / duration  # Messages per second
    
    def run_all_tests(self):
        """Run complete test suite"""
        print("Multi-Protocol Gateway Test Suite")
        print("="*50)
        
        try:
            # Setup
            threads = self.setup_test_environment()
            
            # Run individual tests
            results = {}
            results['can_to_ethernet'] = self.test_can_to_ethernet_routing()
            results['can_to_lin'] = self.test_can_to_lin_routing()
            results['flexray_to_can'] = self.test_flexray_to_can_routing()
            results['ethernet_to_can'] = self.test_ethernet_to_can_routing()
            results['routing_failures'] = self.test_routing_failures()
            results['data_transformation'] = self.test_data_transformation()
            
            # Performance test
            print("\nRunning performance test...")
            throughput = self.run_performance_test(duration=3.0, message_rate=50)
            results['performance'] = throughput > 40  # Expect >40 msg/s throughput
            
            # Wait a moment for all messages to be processed
            time.sleep(1.0)
            
            # Generate reports
            self.gateway.generate_statistics_report()
            
            # Test summary
            print(f"\nTEST SUMMARY:")
            print("-" * 40)
            
            passed_tests = sum(1 for result in results.values() if result)
            total_tests = len(results)
            
            for test_name, result in results.items():
                status = "PASS" if result else "FAIL"
                print(f"{test_name}: {status}")
            
            overall_result = passed_tests == total_tests
            print(f"\nOverall Result: {passed_tests}/{total_tests} tests passed")
            print(f"Gateway Test: {'PASS' if overall_result else 'FAIL'}")
            
        except Exception as e:
            print(f"Test suite failed: {e}")
        
        finally:
            self.gateway.stop_gateway()

def demonstrate_gateway_testing():
    """Main demonstration function"""
    print("Automotive Protocol Gateway Testing Demonstration")
    print("=" * 60)
    
    tester = GatewayTester()
    tester.run_all_tests()

if __name__ == "__main__":
    demonstrate_gateway_testing()