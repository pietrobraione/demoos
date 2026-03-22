# Installation

Before running DemoOS, be sure to install the following packages, which will let
you compile and run the operating system on the virtual machine. 

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

sudo apt-get install dosfstools
sudo apt-get install parted
```

When all the packages will be installed and the virtual disk will be ready, 
you'll have everything you need to run DemoOS. 

``` bash
# Creates the SD virtual disk
make sd
# Compiles everything, generates the kernel binary and copes the app binaries to
# the SD card
make
# Runs the compiled files into a virtual machine in qemu
make run
# Clears the compiled file, so you can run another build from zero (this doesn't
# delete the SD image)
make clean
```
