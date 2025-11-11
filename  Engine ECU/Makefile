# Makefile for Engine ECU Startup Monitoring
# Production-ready AUTOSAR ECU build system

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
INCLUDES = -Isrc -Iconfig
SRCDIR = src
OBJDIR = build
TARGET = engine_ecu

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Default target
all: directories $(TARGET)

# Create build directory
directories:
	@mkdir -p $(OBJDIR)

# Link target
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CC) $(OBJECTS) -o $(TARGET)
	@echo "✓ Build complete: $(TARGET)"

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(OBJDIR) $(TARGET)
	@echo "✓ Clean complete"

# Run the ECU simulation
run: $(TARGET)
	@echo "Running Engine ECU Simulation..."
	@echo "================================"
	./$(TARGET)

# Install development dependencies (if any)
install-deps:
	@echo "Installing development dependencies..."
	# Add any dependency installation commands here
	@echo "✓ Dependencies installed"

# Validate AUTOSAR configurations
validate-config:
	@echo "Validating AUTOSAR configuration files..."
	python3 tools/config_analyzer.py config/
	@echo "✓ Configuration validation complete"

# Run full diagnostic
diagnose: validate-config run
	@echo "Running full ECU diagnostic..."
	./tools/troubleshoot_startup.sh

# Help target
help:
	@echo "Available targets:"
	@echo "  all           - Build the Engine ECU application"
	@echo "  clean         - Clean build artifacts"
	@echo "  run           - Run ECU simulation"
	@echo "  validate-config - Validate AUTOSAR configuration"
	@echo "  diagnose      - Run full diagnostic suite"
	@echo "  install-deps  - Install development dependencies"
	@echo "  help          - Show this help message"

.PHONY: all clean run install-deps validate-config diagnose help directories