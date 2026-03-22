echo "Loading files to the SD card..."

# First I mount the SD image
sudo mkdir -p /mnt/demoos-sd
sudo losetup -fP disk.img
sudo mount /dev/loop0p1 /mnt/demoos-sd

# I copy the binary files from the ./app folder to the SD
sudo mkdir -p /mnt/demoos-sd/bin/
sudo cp ./app/*.bin /mnt/demoos-sd/bin/

# Finally I unmount the image
sudo umount /mnt/demoos-sd
sudo losetup -D

echo "Loading completed!"
