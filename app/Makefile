# Toolchain
CC := aarch64-elf-gcc
LD := aarch64-elf-ld
OBJCOPY := aarch64-elf-objcopy

# Flags
CFLAGS := -ffreestanding -nostdlib -nostartfiles
LDFLAGS := -nostdlib

all: test.bin

test.bin: test.elf 
	$(OBJCOPY) -O binary test.elf test.bin

test.elf: test.o user_syscalls.o
	$(LD) -T test.ld test.o user_syscalls.o -o test.elf

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f test.bin test.elf test.o user_syscalls.o
