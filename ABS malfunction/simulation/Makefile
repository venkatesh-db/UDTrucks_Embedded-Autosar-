# Makefile for ABS Malfunction Detection Simulation

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -g
TARGET = abs_simulation
SRCDIR = .
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:.c=.o)

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	@echo "🔨 Linking $(TARGET)..."
	$(CC) $(OBJECTS) -o $(TARGET) -lm
	@echo "✅ Build complete!"

# Compile source files
%.o: %.c
	@echo "🔧 Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -f $(OBJECTS) $(TARGET)
	@echo "✅ Clean complete!"

# Run the simulation
run: $(TARGET)
	@echo "🚗 Starting ABS Malfunction Detection Simulation..."
	@echo ""
	./$(TARGET)

# Install dependencies (if needed)
install-deps:
	@echo "📦 No additional dependencies required for this simulation"
	@echo "✅ Standard C library and math library (-lm) are sufficient"

# Help
help:
	@echo "🚗 ABS Malfunction Detection Simulation"
	@echo "======================================"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build the simulation executable"
	@echo "  run        - Build and run the simulation"
	@echo "  clean      - Remove build artifacts"
	@echo "  help       - Show this help message"
	@echo ""
	@echo "Usage examples:"
	@echo "  make run   # Build and run simulation"
	@echo "  make clean # Clean build files"

.PHONY: all clean run install-deps help