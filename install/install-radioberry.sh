#!/bin/bash
#if false; then
sudo apt update
sudo apt install build-essential git cmake swig -y
sudo apt-get -y install binutils-dev 
sudo apt-get -y install libdw-dev
sudo apt-get -y install libdwarf-dev
sudo apt-get -y install libdw-dev 
sudo apt-get -y install git
sudo apt-get -y install gfortran
sudo apt-get install -y\
    cmake g++ \
    libpython-dev python-numpy swig
git clone https://github.com/pothosware/SoapySDR.git
cd SoapySDR
git pull origin master
mkdir build
cd build
cmake ..
make -j4
sudo make install
sudo ldconfig
cd ~
#git clone https://github.com/paulh002/rtaudio
#cd rtaudio
#mkdir build
#cd build
#cmake .. -DRTAUDIO_API_ALSA=1
#make
#sudo make install
#sudo ldconfig
#cd ~
sudo apt-get install -y fftw3-dev
git clone https://github.com/jgaeddert/liquid-dsp
sudo apt-get install -y automake autoconf
cd liquid-dsp
git checkout v1.6.0
./bootstrap.sh
./configure
make -j4
sudo make install
sudo ldconfig
cd ~
# wget http://www.kernel.org/pub/linux/bluetooth/bluez-5.60.tar.xz
# tar xvf bluez-5.60.tar.xz
# sudo apt-get install -y libusb-dev libreadline-dev libglib2.0-dev libudev-dev libdbus-1-dev libical-dev
# sudo apt-get install -y docutils-common
# cd bluez-5.60
# ./configure --enable-library
# make -j4
# sudo make install
# cd ~
# git clone https://github.com/intel-iot-devkit/tinyb
# cd tinyb
# mkdir build
# cd build
# cmake ..
# make -j4
# sudo make install
# sudo ldconfig 
# cd ~
sudo apt-get install -y ninja-build libiio-dev libiio-dev libiio-utils libasound-dev
sudo apt-get install -y -f libboost-all-dev
sudo ldconfig
sudo apt-get remove -y pulseaudio
#fi
git clone https://github.com/paulh002/SoapyRadioberry
cd SoapyRadioberry
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
cd ~
#-----------------------------------------------------------------------------
echo "Installing Radioberry driver..."
#git clone  --depth=1 https://github.com/pa3gsb/Radioberry-2.x

sudo apt-get -y install raspberrypi-kernel-headers
sudo apt-get -y install linux-headers-rpi
sudo apt-get -y install device-tree-compiler
sudo apt-get -y install pigpio

#unregister radioberry driver
sudo modprobe -r radioberry
	
if [ ! -d "/lib/modules/$(uname -r)/kernel/drivers/sdr" ]; then
	sudo mkdir /lib/modules/$(uname -r)/kernel/drivers/sdr
fi

#cd Radioberry-2.x/SBC/rpi-4/device_driver/driver
cd SoapyRadioberry/driver
make

sudo cp radioberry.ko /lib/modules/$(uname -r)/kernel/drivers/sdr
sudo dtc -@ -I dts -O dtb -o radioberry.dtbo radioberry.dts
sudo cp radioberry.dtbo /boot/overlays
#add driver to config.txt
sudo grep -Fxq "dtoverlay=radioberry" /boot/config.txt || sudo sed -i '$ a dtoverlay=radioberry' /boot/config.txt
sudo cp ./radioberry.rbf /lib/firmware/.
cd ~

sudo depmod	
#register radioberry driver
sudo modprobe radioberry
sudo chmod 666 /dev/radioberry
#show radioberry driver info.
sudo modinfo radioberry

echo ""
echo "Radioberry driver installed."
#-----------------------------------------------------------------------------
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/sdrberry_settings_radioberry.cfg
mv sdrberry_settings_radioberry.cfg sdrberry_settings.cfg
git clone https://github.com/paulh002/sdrberry
cd sdrberry
mkdir build
cd build
cmake ..
make
sudo make install
cd ~
sudo sed -i 's/$/ vt.global_cursor_default=0 usbhid.mousepoll=2/' /boot/cmdline.txt
sudo sed -i '/dtparam=i2c_arm=on/s/^#//g' /boot/config.txt
sudo reboot