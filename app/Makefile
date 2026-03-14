# Toolchain
CC := aarch64-elf-gcc
LD := aarch64-elf-ld
OBJCOPY := aarch64-elf-objcopy

# Flags
CFLAGS := -ffreestanding -nostdlib -nostartfiles
LDFLAGS := -nostdlib

all: calculator.bin

calculator.bin: calculator.elf 
	$(OBJCOPY) -O binary calculator.elf calculator.bin

calculator.elf: calculator.o
	$(LD) -T app.ld calculator.o ../common/user_syscalls.o -o calculator.elf

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f calculator.bin calculator.elf calculator.o
