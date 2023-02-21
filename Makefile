BUILD_DIR = build
BIN_NAME = template

INCLUDE_DIRS =\
	-Iinclude\
	-Icmsis

SRC_FILES =\
	src/main.c\
	src/startup.c

LINKER_SCRIPT = linker/rp2040.ld

CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
BIN2UF2 = bin2uf2/bin2uf2
ZIG = zig-0.11.0-dev build-exe

ZIGFLAGS = -O ReleaseSafe --gc-sections -ffunction-sections -fstrip

CFLAGS =  -Wall -std=gnu11 -O0
CFLAGS += -ffreestanding
CFLAGS += -fdata-sections -ffunction-sections
CFLAGS += -funsigned-char -funsigned-bitfields
CFLAGS += -mcpu=cortex-m0plus -mthumb
CFLAGS += $(INCLUDE_DIRS)
CFLAGS += -nostartfiles
CFLAGS += -Wl,--gc-sections,--script=$(LINKER_SCRIPT)

all: proj

proj: $(BUILD_DIR)/$(BIN_NAME).uf2

$(BUILD_DIR)/$(BIN_NAME).elf: $(SRC_FILES) $(BUILD_DIR)
	$(CC) $(CFLAGS) $(SRC_FILES) -o $@
	
$(BUILD_DIR)/$(BIN_NAME).bin: $(BUILD_DIR)/$(BIN_NAME).elf
	$(OBJCOPY) -O binary $^ $@

$(BUILD_DIR)/$(BIN_NAME).uf2: $(BUILD_DIR)/$(BIN_NAME).bin $(BIN2UF2)
	$(BIN2UF2) $(BUILD_DIR)/$(BIN_NAME).bin -o $@

$(BIN2UF2): bin2uf2/bin2uf2.zig
	$(ZIG) $(ZIGFLAGS) $^ -femit-bin=$@

$(BUILD_DIR):
	mkdir $(BUILD_DIR)

