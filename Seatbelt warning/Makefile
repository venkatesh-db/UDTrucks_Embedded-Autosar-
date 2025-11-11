CC=clang
CFLAGS=-std=c11 -Wall -Wextra -Werror -O2
LDFLAGS=
BUILD_DIR=build
SRC_DIR=src
APP_DIR=$(SRC_DIR)/application
RTE_DIR=$(SRC_DIR)/rte
BSW_DIR=$(SRC_DIR)/bsw
PLAT_DIR=$(SRC_DIR)/platform
CFG_DIR=$(SRC_DIR)/config

SIM_OBJS=$(BUILD_DIR)/platform/main.o \
          $(BUILD_DIR)/application/Seatbelt_Sensor_IF.o \
          $(BUILD_DIR)/application/Occupancy_Sensor_IF.o \
          $(BUILD_DIR)/application/VehicleState_IF.o \
          $(BUILD_DIR)/application/SeatbeltWarning_Logic.o \
          $(BUILD_DIR)/rte/Rte.o \
          $(BUILD_DIR)/bsw/Dem.o \
          $(BUILD_DIR)/bsw/NvM.o \
          $(BUILD_DIR)/bsw/Crc.o

TEST_OBJS=$(BUILD_DIR)/tests/test_debounce.o \
          $(BUILD_DIR)/application/Seatbelt_Sensor_IF.o \
          $(BUILD_DIR)/application/Occupancy_Sensor_IF.o \
          $(BUILD_DIR)/application/VehicleState_IF.o \
          $(BUILD_DIR)/application/SeatbeltWarning_Logic.o \
          $(BUILD_DIR)/rte/Rte.o \
          $(BUILD_DIR)/bsw/Dem.o \
          $(BUILD_DIR)/bsw/NvM.o \
          $(BUILD_DIR)/bsw/Crc.o

INCLUDES=-I$(SRC_DIR) -I$(APP_DIR) -I$(RTE_DIR) -I$(BSW_DIR) -I$(PLAT_DIR) -I$(CFG_DIR)

all: $(BUILD_DIR)/sim $(BUILD_DIR)/test

$(BUILD_DIR)/sim: $(SIM_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(SIM_OBJS) $(LDFLAGS)

$(BUILD_DIR)/test: $(TEST_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJS) $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR)/tests/%.o: tests/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

run: all
	$(BUILD_DIR)/sim

test: $(BUILD_DIR)/test
	$(BUILD_DIR)/test

clean:
	rm -rf $(BUILD_DIR)

.PHONY: all run clean test
