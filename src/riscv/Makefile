CROSS ?= riscv32-none-elf-

CC = $(CROSS)gcc
OBJCOPY = $(CROSS)objcopy
SIZE = $(CROSS)size
ICEPROG = iceprog

CFLAGS=-Wall -O2 -march=rv32im_zicsr -mabi=ilp32 -ffreestanding -flto -nostartfiles -fomit-frame-pointer -Wl,--gc-section -I.. -lc_nano # --specs=nano.specs

CFLAGS += \
	-DNORMALUNIX \
	$(NULL)


include ../sources.mk

# Filter out d_main, we provide our own simplified one
SOURCES_doom := $(filter-out d_main.c,$(SOURCES_doom))

# Filter out s_sound, we provide a dummy one
SOURCES_doom := $(filter-out s_sound.c,$(SOURCES_doom))


SOURCES_doom_arch := \
	d_main.c \
	i_main.c \
	i_net.c \
	i_sound.c \
	i_system.c \
	i_video.c \
	s_sound.c \
	start.S \
	console.c  \
	libc_backend.c  \
	mini-printf.c \
	$(NULL)


all: doom-riscv.bin

doom-riscv.elf: riscv.lds $(addprefix ../,$(SOURCES_doom)) $(SOURCES_doom_arch)
	$(CC) $(CFLAGS) -Wl,-Bstatic,-T,riscv.lds,--strip-debug -o $@ $(addprefix ../,$(SOURCES_doom)) $(SOURCES_doom_arch)
	$(SIZE) $@

clean:
	rm -f *.bin *.hex *.elf *.o *.gen.h


%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

prog: doom-riscv.bin
	$(ICEPROG) -o 1M $<

prog_wad: data/doomu.wad
	$(ICEPROG) -o 2M $<

load:
	riscv32-none-elf-objcopy -O binary ./doom-riscv.elf ./doom-riscv.bin
	sudo dd if=./doom-riscv.bin of=/dev/sdg
	sync

.PHONY: all clean prog prog_wad
.PRECIOUS: *.elf
