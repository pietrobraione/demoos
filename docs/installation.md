# Installation

Before running DemoOS, be sure to install the following packages, which will let you compile and run the operating system on the virtual machine. 

``` bash
sudo apt-get install build-essential

sudo apt-get install gcc-aarch64-linux-gnu
sudo ln -s $(which aarch64-linux-gnu-gcc) /usr/local/bin/aarch64-elf-gcc

sudo apt-get install binutils-aarch64-linux-gnu
sudo ln -s $(which aarch64-linux-gnu-ld) /usr/local/bin/aarch64-elf-ld

sudo apt-get install binutils-aarch64-linux-gnu
sudo ln -s $(which aarch64-linux-gnu-objcopy) /usr/local/bin/aarch64-elf-objcopy

sudo apt-get install binutils-aarch64-linux-gnu
sudo ln -s $(which aarch64-linux-gnu-ld) /usr/local/bin/aarch64-elf-ld

sudo apt-get install qemu-system-arm
```

You also need to follow these steps to create a virtual disk that will let you use the file system.

``` bash
sudo apt-get install dosfstools
sudo apt-get install parted

dd if=/dev/zero of=disk.img bs=1M count=64

parted disk.img --script mklabel msdos

parted disk.img --script mkpart primary fat32 1MiB 100%

sudo losetup -fP disk.img

lsblk

# Substitute loop0p1 with the device in which the virtual disk has been mounted
sudo mkfs.fat -F 32 /dev/loop0p1

sudo losetup -d /dev/loop0
```

When all the packages will be installed and the virtual disk will be ready, you'll have everything you need to run DemoOS. 

``` bash
# Compiles everything and generates the kernel binary
make
# Runs the compiled files into a virtual machine in qemu
make run
# Clears the compiled file, so you can run another build from zero
make clean
```
