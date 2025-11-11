#!/usr/bin/env python3
"""
Vector CANalyzer Integration Script
Converts our demonstration data to professional Vector formats
"""

import json
import csv
from datetime import datetime
import struct
from pathlib import Path

# Import our demonstration modules
from can_analyzer_demo_simulation import CANAnalyzer
from lin_protocol_demo import LINScheduler
from gateway_testing_demo import ProtocolGateway

class VectorCANalyzerExporter:
    """Export demonstration data to Vector CANalyzer formats"""
    
    def __init__(self):
        self.messages = []
        self.signals = []
        
    def add_can_demo_data(self):
        """Add CAN demonstration messages"""
        # Generate test data from our CAN demo
        test_messages = [
            {'timestamp': 0.000, 'id': 0x123, 'data': [0x01, 0x02, 0x03, 0x04], 'name': 'Engine_Data'},
            {'timestamp': 0.010, 'id': 0x456, 'data': [0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, 0x80], 'name': 'Transmission_Status'},
            {'timestamp': 0.020, 'id': 0x789, 'data': [0xFF, 0x00, 0xAA, 0x55], 'name': 'ABS_Data'},
            {'timestamp': 0.030, 'id': 0x100, 'data': [0x80, 0x40, 0x20, 0x10], 'name': 'Airbag_Status'},
            {'timestamp': 0.040, 'id': 0x200, 'data': [0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC], 'name': 'Body_Control'},
        ]
        
        self.messages.extend(test_messages)
        
        # Define signal interpretations
        self.signals = [
            {'message': 'Engine_Data', 'signal': 'RPM', 'start_bit': 0, 'length': 16},
            {'message': 'Engine_Data', 'signal': 'Throttle', 'start_bit': 16, 'length': 8},
            {'message': 'Transmission_Status', 'signal': 'Gear', 'start_bit': 0, 'length': 4},
            {'message': 'ABS_Data', 'signal': 'WheelSpeed_FL', 'start_bit': 0, 'length': 16},
        ]
    
    def export_to_asc(self, filename="demo_can_traffic.asc"):
        """Export to Vector ASC format"""
        print(f"📁 Exporting to ASC format: {filename}")
        
        with open(filename, 'w') as f:
            # ASC file header
            f.write("date Mon Jan 01 12:00:00.000 2024\n")
            f.write("base hex  timestamps absolute\n")
            f.write("internal events logged\n")
            f.write("// Generated from CAN Protocol Demonstration\n")
            f.write("// Compatible with Vector CANalyzer/CANoe\n")
            f.write("Begin Triggerblock Mon Jan 01 12:00:00.000 2024\n")
            f.write("//   timestamp       channel   ID       direction   length   data\n")
            
            for msg in self.messages:
                # Format: timestamp   channel  ID            direction  d  length  data
                data_str = ' '.join([f'{b:02x}' for b in msg['data']])
                f.write(f"{msg['timestamp']:10.6f}   1  {msg['id']:3x}x             Rx   d {len(msg['data'])} {data_str}\n")
            
            f.write("End TriggerBlock\n")
        
        print(f"✅ ASC file created: {filename}")
        return filename
    
    def export_to_dbc(self, filename="demo_database.dbc"):
        """Export to Vector DBC database format"""
        print(f"📁 Exporting to DBC format: {filename}")
        
        with open(filename, 'w') as f:
            # DBC file header
            f.write('VERSION ""\n\n')
            f.write('NS_ :\n')
            f.write('\tNS_DESC_\n')
            f.write('\tCM_\n')
            f.write('\tBA_DEF_\n')
            f.write('\tBA_\n')
            f.write('\tVAL_\n')
            f.write('\tBA_DEF_DEF_\n')
            f.write('\tEV_DATA_\n')
            f.write('\tENVVAR_DATA_\n')
            f.write('\tSGTYPE_\n')
            f.write('\tSGTYPE_VAL_\n')
            f.write('\tBA_DEF_SGTYPE_\n')
            f.write('\tSIG_VALTYPE_\n')
            f.write('\tSIGTYPE_VALTYPE_\n')
            f.write('\tBO_TX_BU_\n')
            f.write('\tSG_MUL_VAL_\n\n')
            
            f.write('BS_:\n\n')
            
            # Bus units (ECUs)
            f.write('BU_: Engine_ECU Transmission_ECU ABS_ECU Body_ECU\n\n')
            
            # Messages
            unique_messages = {}
            for msg in self.messages:
                if msg['id'] not in unique_messages:
                    unique_messages[msg['id']] = msg
            
            for msg_id, msg in unique_messages.items():
                f.write(f"BO_ {msg_id} {msg['name']}: {len(msg['data'])} Engine_ECU\n")
                
                # Add signals for this message
                msg_signals = [s for s in self.signals if s['message'] == msg['name']]
                for signal in msg_signals:
                    f.write(f" SG_ {signal['signal']} : {signal['start_bit']}|{signal['length']}@1+ (1,0) [0|0] \"\" Engine_ECU\n")
                
                f.write("\n")
            
            # Comments for messages
            f.write('CM_ BO_ 291 "Engine control data including RPM and throttle position";\n')
            f.write('CM_ BO_ 1110 "Transmission status and gear information";\n')
            f.write('CM_ BO_ 1929 "ABS system data with wheel speeds";\n')
        
        print(f"✅ DBC file created: {filename}")
        return filename
    
    def export_to_blf(self, filename="demo_log.blf"):
        """Export to Vector BLF (Binary Logging Format)"""
        print(f"📁 Exporting to BLF format: {filename}")
        
        # BLF is a complex binary format - this is a simplified version
        # For production use, consider using Vector's official tools or libraries
        
        try:
            with open(filename, 'wb') as f:
                # BLF file signature
                f.write(b'LOGG')
                
                # Simple header (simplified)
                header = struct.pack('<IIII', 0x1, 0x0, len(self.messages), 0x0)
                f.write(header)
                
                # Write messages in simplified binary format
                for msg in self.messages:
                    # Timestamp (8 bytes), ID (4 bytes), Length (1 byte), Data (up to 8 bytes)
                    timestamp_ns = int(msg['timestamp'] * 1_000_000_000)  # Convert to nanoseconds
                    
                    msg_header = struct.pack('<QIB', timestamp_ns, msg['id'], len(msg['data']))
                    f.write(msg_header)
                    
                    # Pad data to 8 bytes
                    data_padded = msg['data'] + [0] * (8 - len(msg['data']))
                    f.write(struct.pack('<8B', *data_padded[:8]))
            
            print(f"✅ BLF file created: {filename}")
            print("   ⚠️  Note: This is a simplified BLF format")
            print("   ⚠️  For production use, use Vector's official BLF libraries")
            
        except Exception as e:
            print(f"❌ Error creating BLF file: {e}")
        
        return filename

class CANoeTestConfiguration:
    """Generate Vector CANoe test configurations"""
    
    def __init__(self):
        self.test_cases = []
        
    def add_arbitration_test(self):
        """Add CAN arbitration test case"""
        test = {
            'name': 'CAN_Arbitration_Test',
            'description': 'Verify CAN arbitration mechanism with simultaneous transmissions',
            'setup': [
                'Configure two nodes with different priority messages',
                'Node A sends ID 0x100 (high priority)',
                'Node B sends ID 0x700 (low priority)',
            ],
            'execution': [
                'Trigger simultaneous transmission',
                'Monitor bus for arbitration behavior',
                'Verify high priority message wins',
            ],
            'expected_results': [
                'Message 0x100 transmitted successfully',
                'Message 0x700 deferred and retransmitted',
                'No arbitration errors detected',
            ]
        }
        self.test_cases.append(test)
    
    def add_error_handling_test(self):
        """Add error handling test case"""
        test = {
            'name': 'CAN_Error_Handling_Test',
            'description': 'Verify CAN error detection and recovery mechanisms',
            'setup': [
                'Configure normal CAN communication',
                'Prepare error injection capabilities',
            ],
            'execution': [
                'Inject bit errors during transmission',
                'Monitor error counters (TEC/REC)',
                'Verify error frame generation',
                'Test error recovery behavior',
            ],
            'expected_results': [
                'Bit errors detected and flagged',
                'Error frames generated correctly',
                'Automatic retransmission occurs',
                'Error counters increment appropriately',
            ]
        }
        self.test_cases.append(test)
    
    def export_test_configuration(self, filename="demo_test_config.json"):
        """Export test configuration to JSON format"""
        print(f"📁 Exporting CANoe test configuration: {filename}")
        
        config = {
            'project_name': 'CAN_Protocol_Demo_Tests',
            'version': '1.0',
            'created': datetime.now().isoformat(),
            'description': 'Test configuration generated from CAN protocol demonstrations',
            'test_cases': self.test_cases,
            'network_configuration': {
                'baudrate': 500000,
                'sample_point': 0.875,
                'sjw': 1,
                'protocols': ['CAN', 'LIN', 'FlexRay']
            }
        }
        
        with open(filename, 'w') as f:
            json.dump(config, f, indent=2)
        
        print(f"✅ Test configuration exported: {filename}")
        return filename

def demonstrate_vector_integration():
    """Main demonstration of Vector tool integration"""
    print("🔧 Vector CANalyzer/CANoe Integration Demonstration")
    print("=" * 60)
    
    # Create exporter and add demo data
    exporter = VectorCANalyzerExporter()
    exporter.add_can_demo_data()
    
    # Export to various Vector formats
    print("\n1️⃣ Exporting trace data...")
    asc_file = exporter.export_to_asc()
    
    print("\n2️⃣ Exporting database definition...")
    dbc_file = exporter.export_to_dbc()
    
    print("\n3️⃣ Exporting binary log...")
    blf_file = exporter.export_to_blf()
    
    # Create test configuration
    print("\n4️⃣ Creating test configuration...")
    test_config = CANoeTestConfiguration()
    test_config.add_arbitration_test()
    test_config.add_error_handling_test()
    config_file = test_config.export_test_configuration()
    
    # Generate usage instructions
    print("\n📋 Integration Instructions:")
    print("=" * 40)
    print("1. Vector CANalyzer Integration:")
    print(f"   • Open CANalyzer")
    print(f"   • Load database: {dbc_file}")
    print(f"   • Import trace: {asc_file}")
    print(f"   • Analyze message timing and content")
    
    print("\n2. Vector CANoe Integration:")
    print(f"   • Create new CANoe configuration")
    print(f"   • Import database: {dbc_file}")
    print(f"   • Load test configuration: {config_file}")
    print(f"   • Run automated tests")
    
    print("\n3. Professional Analysis Steps:")
    print("   • Message timing analysis")
    print("   • Bus load calculation")
    print("   • Error rate assessment")
    print("   • Protocol compliance verification")
    
    print(f"\n✅ Vector integration files ready!")
    print(f"   Files created: {asc_file}, {dbc_file}, {blf_file}, {config_file}")

def create_pcan_integration_script():
    """Create integration script for PCAN hardware"""
    pcan_script = '''
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
'''
    
    with open('PCAN_Integration_Guide.md', 'w') as f:
        f.write(pcan_script)
    
    print("📄 Created PCAN_Integration_Guide.md")

if __name__ == "__main__":
    demonstrate_vector_integration()
    create_pcan_integration_script()