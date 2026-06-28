#!/bin/bash
sudo apt-get install -y flex bison aptitude libssl-dev
git clone --depth=1 --branch rpi-6.12.y https://github.com/raspberrypi/linux
cd linux
curl -L https://github.com/raspberrypi/linux/pull/6927.diff -o pr.diff
git apply pr.diff
# 1. Change the max_burst value
sed -i 's/dma_caps\.max_burst = 4;/dma_caps.max_burst = 2;/' drivers/misc/rp1-pio.c
# 2. Comment out the dma_get_slave_caps call
sed -i 's/^\([[:space:]]*\)dma_get_slave_caps(dma->chan, \&dma_caps);/\1\/\/ dma_get_slave_caps(dma->chan, \&dma_caps);/' drivers/misc/rp1-pio.c
make clean
make mrproper
sudo make ARCH=arm64 bcm2712_defconfig
sudo make ARCH=arm64 -j"$(nproc)" Image modules dtbs
sudo cp arch/arm64/boot/Image /boot/firmware/kernel_2712.img
sudo cp arch/arm64/boot/dts/broadcom/*.dtb /boot/firmware/
sudo cp arch/arm64/boot/dts/overlays/*.dtb* /boot/firmware/overlays/
sudo cp arch/arm64/boot/dts/overlays/README /boot/firmware/overlays/
sudo make ARCH=arm64 INSTALL_MOD_PATH=/ INSTALL_MOD_STRIP=1 modules_install
sudo reboot
