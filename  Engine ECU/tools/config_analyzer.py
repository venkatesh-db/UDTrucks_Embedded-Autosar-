# AUTOSAR Configuration Analysis Script
# Checks common configuration issues that prevent ECU startup

import xml.etree.ElementTree as ET
import os
import sys
from pathlib import Path

class AutosarConfigAnalyzer:
    def __init__(self, config_path):
        self.config_path = Path(config_path)
        self.issues = []
        
    def analyze_ecu_configuration(self):
        """Analyze ECU-C configuration files"""
        print("Analyzing AUTOSAR ECU Configuration...")
        
        # Check for essential configuration files
        required_files = [
            'EcuC.arxml',
            'Os.arxml', 
            'Rte.arxml',
            'CanIf.arxml',
            'Com.arxml'
        ]
        
        for file in required_files:
            file_path = self.config_path / file
            if not file_path.exists():
                self.issues.append(f"Missing configuration file: {file}")
            else:
                self.validate_config_file(file_path)
    
    def validate_config_file(self, file_path):
        """Validate individual ARXML configuration file"""
        try:
            tree = ET.parse(file_path)
            root = tree.getroot()
            
            # Check for basic XML structure
            if root.tag != '{http://autosar.org/schema/r4.0}AUTOSAR':
                self.issues.append(f"Invalid AUTOSAR XML structure in {file_path.name}")
            
            # Check for empty configurations
            if len(root) == 0:
                self.issues.append(f"Empty configuration file: {file_path.name}")
                
        except ET.ParseError as e:
            self.issues.append(f"XML parse error in {file_path.name}: {str(e)}")
        except Exception as e:
            self.issues.append(f"Error reading {file_path.name}: {str(e)}")
    
    def check_memory_layout(self):
        """Check memory section configuration"""
        print("Checking memory layout configuration...")
        
        # Common memory issues
        memory_checks = [
            "Flash memory sections overlap",
            "RAM sections exceed available memory", 
            "Stack size insufficient for application",
            "Heap configuration missing or too small"
        ]
        
        # TODO: Implement actual memory layout validation
        # This would require parsing linker scripts and memory maps
        
    def check_clock_configuration(self):
        """Verify clock and timing configuration"""
        print("Verifying clock configuration...")
        
        # Clock configuration issues
        clock_issues = [
            "PLL configuration invalid for target frequency",
            "Peripheral clock frequencies exceed maximum",
            "CAN baud rate configuration mismatch",
            "OS tick frequency not configured"
        ]
        
        # TODO: Implement clock validation logic
        
    def check_communication_config(self):
        """Check CAN/LIN communication configuration"""
        print("Checking communication configuration...")
        
        # Communication stack issues
        comm_checks = [
            "CAN controller not properly configured",
            "Message ID conflicts in CAN database",
            "Missing network management configuration",
            "Diagnostic service configuration incomplete"
        ]
        
        # TODO: Implement communication validation
        
    def generate_report(self):
        """Generate analysis report"""
        print("\n" + "="*60)
        print("AUTOSAR Configuration Analysis Report")
        print("="*60)
        
        if not self.issues:
            print("✅ No configuration issues detected!")
        else:
            print(f"❌ Found {len(self.issues)} potential issues:")
            for i, issue in enumerate(self.issues, 1):
                print(f"{i}. {issue}")
        
        print("\n" + "="*60)
        
        # Recommendations
        print("\nRecommendations for ECU startup issues:")
        print("1. Verify bootloader compatibility with new software")
        print("2. Check memory layout for overlaps or overruns") 
        print("3. Validate clock configuration matches hardware")
        print("4. Ensure all required AUTOSAR modules are configured")
        print("5. Test communication interfaces individually")
        print("6. Review application startup sequence")
        
    def run_analysis(self):
        """Run complete configuration analysis"""
        self.analyze_ecu_configuration()
        self.check_memory_layout()
        self.check_clock_configuration()
        self.check_communication_config()
        self.generate_report()

if __name__ == "__main__":
    config_path = "../config"  # Default config directory
    
    if len(sys.argv) > 1:
        config_path = sys.argv[1]
    
    analyzer = AutosarConfigAnalyzer(config_path)
    analyzer.run_analysis()