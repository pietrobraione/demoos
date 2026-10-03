dd if=/dev/zero of=disk.img bs=1M count=64
sudo parted disk.img --script mklabel msdos
sudo parted disk.img --script mkpart primary fat32 1MiB 100%
sudo losetup -fP disk.img
lsblk
sudo mkfs.fat -F 32 /dev/loop0p1
sudo losetup -d /dev/loop0
