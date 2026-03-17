# First I mount the SD image
sudo mkdir -p /mnt/demoos-sd
sudo losetup -fP disk.img
sudo mount /dev/loop0p1 /mnt/demoos-sd

# I copy the binary files from the ./app folder to the SD
sudo cp ./app/*.bin /mnt/demoos-sd/

# Finally I unmount the image
sudo umount /mnt/demoos-sd
sudo losetup -D
