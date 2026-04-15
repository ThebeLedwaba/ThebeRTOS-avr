# Project Configuration
MCU = atmega328p
F_CPU = 16000000UL
DEVICE = /dev/ttyUSB0  # Update to your port (e.g. COM3 or /dev/ttyACM0)
BAUD = 115200

# Directory Structure
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# Source Files
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/kernel/kernel.c \
          $(SRC_DIR)/kernel/list.c \
          $(SRC_DIR)/kernel/sync.c \
          $(SRC_DIR)/kernel/sync_isr.c \
          $(SRC_DIR)/kernel/sw_timer.c \
          $(SRC_DIR)/kernel/queue.c \
          $(SRC_DIR)/hal/timer.c \
          $(SRC_DIR)/hal/uart.c

ASMS = $(SRC_DIR)/kernel/context.S

# Compiler Configuration
CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size
CFLAGS = -mmcu=$(MCU) -DF_CPU=$(F_CPU) -I$(INC_DIR) -Os -Wall -Wextra -std=gnu99
LDFLAGS = -mmcu=$(MCU)

# Output Files
TARGET = $(BUILD_DIR)/mini_rtos
HEX = $(TARGET).hex

# Objects
OBJECTS = $(SOURCES:%.c=$(BUILD_DIR)/%.o) $(ASMS:%.S=$(BUILD_DIR)/%.o)

all: $(BUILD_DIR) $(HEX)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)/src/kernel
	mkdir -p $(BUILD_DIR)/src/hal

$(HEX): $(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@
	$(SIZE) --format=avr --mcu=$(MCU) $<

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD_DIR)/%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

# Flash using Avrdude (Arduino as ISP or Serial)
flash: $(HEX)
	avrdude -v -p $(MCU) -c arduino -P $(DEVICE) -b $(BAUD) -D -U flash:w:$<:i

.PHONY: all clean flash
