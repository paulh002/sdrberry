#!/bin/bash
cd ~
sudo modprobe -r radioberry
rm -rf Radioberry-2.x
git clone  --depth=1 https://github.com/paulh002/Radioberry-2.x
cd Radioberry-2.x/SBC/rpi-5/device_driver/pio-mode/driver
sed -i '/KBUILD_EXTRA_SYMBOLS := \/home\/pi\/git\/rp1-driver\/Module\.symvers/d' Makefile
make -j4
sudo mkdir /lib/modules/$(uname -r)/kernel/drivers/sdr
sudo cp radioberry.ko /lib/modules/$(uname -r)/kernel/drivers/sdr
sudo cp radioberry.dtbo /boot/overlays
sudo depmod
sudo cp ~/sdrberry/install/trixie/radioberry.rbf /lib/firmware
sudo modprobe radioberry
cd ~
