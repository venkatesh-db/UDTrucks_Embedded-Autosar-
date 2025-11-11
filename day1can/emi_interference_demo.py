#!/usr/bin/env python3
"""
EMI/EMC Interference Testing Simulation
Demonstrates how EMI affects vehicle communication protocols
"""

import random
import time
import math
import threading
from collections import defaultdict
from datetime import datetime

class InterferenceSource:
    """Represents different sources of electromagnetic interference"""
    
    def __init__(self, name, frequency_range, amplitude_range, pattern='random'):
        self.name = name
        self.frequency_min, self.frequency_max = frequency_range
        self.amplitude_min, self.amplitude_max = amplitude_range
        self.pattern = pattern
        self.active = False
        self.current_amplitude = 0
        self.current_frequency = 0
    
    def generate_interference(self, timestamp):
        """Generate interference signal at given timestamp"""
        if not self.active:
            return 0
        
        # Update interference parameters based on pattern
        if self.pattern == 'random':
            self.current_amplitude = random.uniform(self.amplitude_min, self.amplitude_max)
            self.current_frequency = random.uniform(self.frequency_min, self.frequency_max)
        
        elif self.pattern == 'periodic':
            # Sinusoidal variation
            cycle_period = 1.0  # 1 second cycle
            phase = (timestamp % cycle_period) / cycle_period * 2 * math.pi
            amplitude_factor = (math.sin(phase) + 1) / 2  # 0 to 1
            
            self.current_amplitude = self.amplitude_min + \
                                   (self.amplitude_max - self.amplitude_min) * amplitude_factor
            self.current_frequency = (self.frequency_min + self.frequency_max) / 2
        
        elif self.pattern == 'burst':
            # Intermittent bursts
            burst_probability = 0.1  # 10% chance per sample
            if random.random() < burst_probability:
                self.current_amplitude = self.amplitude_max
                self.current_frequency = self.frequency_max
            else:
                self.current_amplitude = 0
                self.current_frequency = 0
        
        # Generate interference signal
        interference_signal = self.current_amplitude * math.sin(
            2 * math.pi * self.current_frequency * timestamp
        )
        
        return interference_signal
    
    def start(self):
        """Start interference source"""
        self.active = True
    
    def stop(self):
        """Stop interference source"""
        self.active = False

class ProtocolSimulator:
    """Simulates vehicle communication protocol under interference"""
    
    def __init__(self, protocol_name, bit_rate, voltage_levels):
        self.protocol_name = protocol_name
        self.bit_rate = bit_rate  # bits per second
        self.voltage_high, self.voltage_low = voltage_levels
        self.noise_threshold = abs(self.voltage_high - self.voltage_low) * 0.1  # 10% of signal swing
        
        # Protocol-specific parameters
        self.bit_duration = 1.0 / bit_rate
        self.current_message = []
        self.transmission_active = False
        
        # Error tracking
        self.bit_errors = 0
        self.frame_errors = 0
        self.total_bits_sent = 0
        self.total_frames_sent = 0
        
        # Interference tracking
        self.interference_samples = []
        self.signal_samples = []
    
    def set_interference_sources(self, sources):
        """Set interference sources affecting this protocol"""
        self.interference_sources = sources
    
    def transmit_bit(self, bit_value, timestamp):
        """Transmit a single bit and check for interference effects"""
        # Determine signal voltage
        signal_voltage = self.voltage_high if bit_value else self.voltage_low
        
        # Calculate total interference
        total_interference = sum(
            source.generate_interference(timestamp) 
            for source in self.interference_sources
        )
        
        # Combine signal and interference
        received_voltage = signal_voltage + total_interference
        
        # Store samples for analysis
        self.signal_samples.append(signal_voltage)
        self.interference_samples.append(total_interference)
        
        # Determine if bit was corrupted
        if bit_value == 1:  # High bit
            corrupted = received_voltage < (self.voltage_high - self.noise_threshold)
        else:  # Low bit
            corrupted = received_voltage > (self.voltage_low + self.noise_threshold)
        
        self.total_bits_sent += 1
        if corrupted:
            self.bit_errors += 1
        
        return not corrupted, received_voltage
    
    def transmit_frame(self, frame_data, timestamp):
        """Transmit a complete frame"""
        self.transmission_active = True
        frame_success = True
        bit_errors_in_frame = 0
        
        # Convert frame data to bits (simplified)
        bits = []
        for byte in frame_data:
            for i in range(8):
                bits.append((byte >> i) & 1)
        
        # Add protocol-specific overhead bits
        if self.protocol_name == 'CAN':
            # Add SOF, arbitration, control, CRC, ACK, EOF
            overhead_bits = [0] + [0, 1] * 11 + [0, 1] * 6 + [1, 0] * 8 + [1, 1] + [1] * 7
            bits = overhead_bits + bits
        elif self.protocol_name == 'LIN':
            # Add sync break, sync field, PID
            overhead_bits = [0] * 13 + [0, 1, 0, 1, 0, 1, 0, 1] + [0, 1] * 4
            bits = overhead_bits + bits
        
        # Transmit each bit
        for i, bit in enumerate(bits):
            bit_timestamp = timestamp + (i * self.bit_duration)
            success, voltage = self.transmit_bit(bit, bit_timestamp)
            
            if not success:
                frame_success = False
                bit_errors_in_frame += 1
        
        self.total_frames_sent += 1
        if not frame_success:
            self.frame_errors += 1
        
        self.transmission_active = False
        return frame_success, bit_errors_in_frame
    
    def get_error_statistics(self):
        """Get current error statistics"""
        ber = self.bit_errors / self.total_bits_sent if self.total_bits_sent > 0 else 0
        fer = self.frame_errors / self.total_frames_sent if self.total_frames_sent > 0 else 0
        
        return {
            'bit_error_rate': ber,
            'frame_error_rate': fer,
            'total_bit_errors': self.bit_errors,
            'total_frame_errors': self.frame_errors,
            'total_bits': self.total_bits_sent,
            'total_frames': self.total_frames_sent
        }
    
    def reset_statistics(self):
        """Reset error statistics"""
        self.bit_errors = 0
        self.frame_errors = 0
        self.total_bits_sent = 0
        self.total_frames_sent = 0
        self.interference_samples.clear()
        self.signal_samples.clear()

class EMITestSuite:
    """Test suite for EMI/EMC interference testing"""
    
    def __init__(self):
        self.interference_sources = self._create_interference_sources()
        self.protocols = self._create_protocol_simulators()
        self.test_results = {}
    
    def _create_interference_sources(self):
        """Create typical automotive interference sources"""
        sources = [
            InterferenceSource(
                name="Mobile Phone GSM",
                frequency_range=(900e6, 1800e6),  # GSM bands
                amplitude_range=(0.1, 0.5),      # Volts
                pattern='burst'
            ),
            InterferenceSource(
                name="Switching Regulator",
                frequency_range=(100e3, 2e6),    # Typical switching frequencies
                amplitude_range=(0.05, 0.2),
                pattern='periodic'
            ),
            InterferenceSource(
                name="Ignition System",
                frequency_range=(10e6, 100e6),   # Broadband noise
                amplitude_range=(0.2, 1.0),
                pattern='random'
            ),
            InterferenceSource(
                name="Electric Motor",
                frequency_range=(1e3, 100e3),    # Motor commutation
                amplitude_range=(0.1, 0.3),
                pattern='periodic'
            ),
            InterferenceSource(
                name="LED Lighting",
                frequency_range=(1e3, 10e3),     # PWM frequency
                amplitude_range=(0.02, 0.1),
                pattern='periodic'
            )
        ]
        return sources
    
    def _create_protocol_simulators(self):
        """Create vehicle protocol simulators"""
        protocols = [
            ProtocolSimulator("CAN", 500e3, (2.5, 1.5)),      # 500 kbps CAN
            ProtocolSimulator("CAN-FD", 2e6, (2.5, 1.5)),     # 2 Mbps CAN-FD
            ProtocolSimulator("LIN", 19.2e3, (12.0, 0.0)),    # 19.2 kbps LIN
            ProtocolSimulator("FlexRay", 10e6, (2.5, 0.0)),   # 10 Mbps FlexRay
        ]
        
        # Assign interference sources to each protocol
        for protocol in protocols:
            protocol.set_interference_sources(self.interference_sources)
        
        return protocols
    
    def test_baseline_performance(self):
        """Test protocol performance without interference"""
        print("Testing baseline performance (no interference)...")
        
        baseline_results = {}
        
        for protocol in self.protocols:
            protocol.reset_statistics()
            
            # Transmit test frames
            test_frames = [
                [0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08],
                [0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80],
                [0xFF, 0x00, 0xAA, 0x55, 0xF0, 0x0F, 0xCC, 0x33]
            ]
            
            for frame in test_frames:
                timestamp = time.time()
                protocol.transmit_frame(frame, timestamp)
                time.sleep(0.01)  # Small delay between frames
            
            baseline_results[protocol.protocol_name] = protocol.get_error_statistics()
        
        self.test_results['baseline'] = baseline_results
        
        # Display baseline results
        print("\nBaseline Performance Results:")
        for protocol_name, stats in baseline_results.items():
            print(f"{protocol_name}: BER={stats['bit_error_rate']:.2e}, "
                  f"FER={stats['frame_error_rate']:.2e}")
    
    def test_interference_scenarios(self):
        """Test protocols under different interference scenarios"""
        scenarios = [
            {
                'name': 'Mobile Phone Interference',
                'sources': ['Mobile Phone GSM'],
                'description': 'Simulates GSM phone near vehicle electronics'
            },
            {
                'name': 'Power Electronics Interference',
                'sources': ['Switching Regulator', 'Electric Motor'],
                'description': 'Simulates power system interference'
            },
            {
                'name': 'Ignition Interference',
                'sources': ['Ignition System'],
                'description': 'Simulates engine ignition system interference'
            },
            {
                'name': 'Multiple Source Interference',
                'sources': ['Mobile Phone GSM', 'Switching Regulator', 'LED Lighting'],
                'description': 'Simulates multiple simultaneous interference sources'
            }
        ]
        
        for scenario in scenarios:
            print(f"\nTesting: {scenario['name']}")
            print(f"Description: {scenario['description']}")
            
            # Activate selected interference sources
            for source in self.interference_sources:
                if source.name in scenario['sources']:
                    source.start()
                else:
                    source.stop()
            
            scenario_results = {}
            
            # Test each protocol under this interference scenario
            for protocol in self.protocols:
                protocol.reset_statistics()
                
                # Transmit test frames under interference
                test_frames = [
                    [0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08],
                    [0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80],
                    [0xFF, 0x00, 0xAA, 0x55, 0xF0, 0x0F, 0xCC, 0x33],
                    [0x00] * 8,  # All zeros test
                    [0xFF] * 8   # All ones test
                ]
                
                for frame in test_frames:
                    timestamp = time.time()
                    success, bit_errors = protocol.transmit_frame(frame, timestamp)
                    time.sleep(0.01)
                
                scenario_results[protocol.protocol_name] = protocol.get_error_statistics()
            
            # Store results
            self.test_results[scenario['name']] = scenario_results
            
            # Display scenario results
            print("Results:")
            for protocol_name, stats in scenario_results.items():
                print(f"  {protocol_name}: BER={stats['bit_error_rate']:.2e}, "
                      f"FER={stats['frame_error_rate']:.2e}")
        
        # Stop all interference sources
        for source in self.interference_sources:
            source.stop()
    
    def test_interference_immunity_levels(self):
        """Test protocol immunity to varying interference levels"""
        print("\nTesting interference immunity levels...")
        
        # Use switching regulator as test source
        test_source = next(s for s in self.interference_sources if s.name == "Switching Regulator")
        
        # Test different interference levels
        interference_levels = [0.05, 0.1, 0.15, 0.2, 0.25, 0.3, 0.4, 0.5]
        
        immunity_results = defaultdict(list)
        
        for level in interference_levels:
            print(f"  Testing interference level: {level}V")
            
            # Set interference level
            test_source.amplitude_min = level
            test_source.amplitude_max = level
            test_source.start()
            
            for protocol in self.protocols:
                protocol.reset_statistics()
                
                # Transmit test frame
                test_frame = [0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA]
                timestamp = time.time()
                protocol.transmit_frame(test_frame, timestamp)
                
                stats = protocol.get_error_statistics()
                immunity_results[protocol.protocol_name].append({
                    'interference_level': level,
                    'bit_error_rate': stats['bit_error_rate'],
                    'frame_error_rate': stats['frame_error_rate']
                })
        
        test_source.stop()
        self.test_results['immunity_levels'] = immunity_results
        
        # Find immunity thresholds (where BER > 1e-6)
        print("\nInterference Immunity Thresholds:")
        for protocol_name, results in immunity_results.items():
            threshold = None
            for result in results:
                if result['bit_error_rate'] > 1e-6:
                    threshold = result['interference_level']
                    break
            
            if threshold:
                print(f"  {protocol_name}: {threshold}V (BER > 1e-6)")
            else:
                print(f"  {protocol_name}: > {interference_levels[-1]}V (very robust)")
    
    def generate_comprehensive_report(self):
        """Generate comprehensive EMI test report"""
        print("\n" + "="*70)
        print("EMI/EMC INTERFERENCE TEST REPORT")
        print("="*70)
        
        # Summary table
        print(f"\nTEST SUMMARY:")
        print("-" * 50)
        
        # Compare baseline vs interference scenarios
        if 'baseline' in self.test_results:
            baseline = self.test_results['baseline']
            
            print(f"{'Protocol':<12} {'Baseline BER':<12} {'Max BER':<12} {'Degradation':<12}")
            print("-" * 50)
            
            for protocol_name in baseline:
                baseline_ber = baseline[protocol_name]['bit_error_rate']
                
                # Find maximum BER from all interference scenarios
                max_ber = baseline_ber
                for scenario_name, results in self.test_results.items():
                    if (scenario_name not in ['baseline', 'immunity_levels'] and 
                        isinstance(results, dict) and 
                        protocol_name in results):
                        scenario_ber = results[protocol_name]['bit_error_rate']
                        max_ber = max(max_ber, scenario_ber)
                
                degradation = max_ber / baseline_ber if baseline_ber > 0 else float('inf')
                
                print(f"{protocol_name:<12} {baseline_ber:<12.2e} {max_ber:<12.2e} {degradation:<12.2f}x")
        
        # Detailed scenario analysis
        print(f"\nDETAILED SCENARIO ANALYSIS:")
        print("-" * 50)
        
        for scenario_name, results in self.test_results.items():
            if scenario_name in ['baseline', 'immunity_levels']:
                continue
            
            print(f"\n{scenario_name}:")
            for protocol_name, stats in results.items():
                ber = stats['bit_error_rate']
                fer = stats['frame_error_rate']
                status = "PASS" if ber < 1e-6 else "FAIL"
                
                print(f"  {protocol_name}: BER={ber:.2e}, FER={fer:.2e} [{status}]")
        
        # Immunity analysis
        if 'immunity_levels' in self.test_results:
            print(f"\nINTERFERENCE IMMUNITY ANALYSIS:")
            print("-" * 50)
            
            immunity_data = self.test_results['immunity_levels']
            for protocol_name, results in immunity_data.items():
                print(f"\n{protocol_name} Immunity Curve:")
                if isinstance(results, list) and results:
                    for result in results[::2]:  # Show every other point
                        level = result['interference_level']
                        ber = result['bit_error_rate']
                        print(f"  {level}V: BER={ber:.2e}")
        
        # Recommendations
        print(f"\nRECOMMENDATIONS:")
        print("-" * 50)
        
        print("1. Protocol Selection:")
        if 'baseline' in self.test_results:
            # Rank protocols by robustness
            protocol_rankings = []
            for scenario_name, results in self.test_results.items():
                if scenario_name in ['baseline', 'immunity_levels']:
                    continue
                
                for protocol_name, stats in results.items():
                    ber = stats['bit_error_rate']
                    protocol_rankings.append((protocol_name, ber))
            
            # Group by protocol and find average BER
            protocol_avg_ber = defaultdict(list)
            for protocol, ber in protocol_rankings:
                protocol_avg_ber[protocol].append(ber)
            
            for protocol in protocol_avg_ber:
                avg_ber = sum(protocol_avg_ber[protocol]) / len(protocol_avg_ber[protocol])
                protocol_avg_ber[protocol] = avg_ber
            
            # Sort by robustness (lower BER is better)
            if protocol_avg_ber:
                sorted_protocols = sorted(protocol_avg_ber.items(), key=lambda x: x[1])
                
                print("   Recommended protocol order (most robust first):")
                for i, (protocol, avg_ber) in enumerate(sorted_protocols, 1):
                    print(f"   {i}. {protocol} (avg BER: {avg_ber:.2e})")
            else:
                print("   No protocol ranking data available.")
        
        print("\n2. EMI Mitigation Strategies:")
        print("   - Use shielded cables for high-speed protocols")
        print("   - Implement proper grounding and isolation")
        print("   - Add filtering at protocol interfaces")
        print("   - Consider protocol-specific error correction")
        print("   - Validate EMC compliance through standardized testing")
        
        print("\n3. Test Validation:")
        print("   - Perform conducted and radiated immunity testing")
        print("   - Test under real vehicle operating conditions")
        print("   - Validate with actual interference sources")
        print("   - Implement continuous monitoring in production")

def demonstrate_emi_testing():
    """Main EMI testing demonstration"""
    print("EMI/EMC Interference Testing Demonstration")
    print("=" * 60)
    
    # Create test suite
    test_suite = EMITestSuite()
    
    try:
        # Run baseline performance test
        test_suite.test_baseline_performance()
        
        # Run interference scenario tests
        test_suite.test_interference_scenarios()
        
        # Run immunity level tests
        test_suite.test_interference_immunity_levels()
        
        # Generate comprehensive report
        test_suite.generate_comprehensive_report()
        
    except Exception as e:
        print(f"EMI testing failed: {e}")

if __name__ == "__main__":
    demonstrate_emi_testing()