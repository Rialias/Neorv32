# Include the nanopb provided Makefile rules
#include /home/ralias/nanopb/extra/nanopb.mk

# Compiler flags to enable all warnings & debug info
#CFLAGS = -ansi -Wall -Werror -g -O0
#CFLAGS += -I$(NANOPB_DIR)

#all: server

#.SUFFIXES:

#clean:
#	rm -f server fileproto.pb.h

#%: %.c fileproto.pb.c
#	$(CC) $(CFLAGS) -o $@ $^ $(NANOPB_CORE)

# Override the default CPU ISA
MARCH = rv32im_zicsr_zifencei

# Override the default RISC-V GCC prefix
#RISCV_PREFIX ?= riscv-none-elf-

# Override default optimization goal
EFFORT = -Os

# Add extended debug symbols
USER_FLAGS += -ggdb -gdwarf-3

# Adjust processor IMEM size
USER_FLAGS += -Wl,--defsym,__neorv32_rom_size=16k

# Adjust processor DMEM size
USER_FLAGS += -Wl,--defsym,__neorv32_ram_size=8k

# Adjust maximum heap size
#USER_FLAGS += -Wl,--defsym,__neorv32_heap_size=1k

# Additional sources
#APP_SRC += $(wildcard ./*.c)
#APP_INC += -I .

# Set path to NEORV32 root directory
NEORV32_HOME ?= ../../../..

# Include the main NEORV32 makefile
include $(NEORV32_HOME)/sw/common/common.mk
