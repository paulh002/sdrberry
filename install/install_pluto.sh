#!/bin/bash
#if false; then
sudo apt update
sudo apt install build-essential git cmake swig -y
sudo apt-get install binutils-dev
sudo apt-get install libdw-dev
sudo apt-get -y install git
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
sudo apt-get install -y fftw3-dev
git clone https://github.com/jgaeddert/liquid-dsp
sudo apt-get install -y automake autoconf
cd liquid-dsp
./bootstrap.sh
./configure
make -j4
sudo make install
sudo ldconfig
cd ~
git clone https://github.com/paulh002/rtaudio
cd rtaudio
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
cd ~
wget http://www.kernel.org/pub/linux/bluetooth/bluez-5.60.tar.xz
tar xvf bluez-5.60.tar.xz
sudo apt-get install -y libusb-dev libreadline-dev libglib2.0-dev libudev-dev libdbus-1-dev libical-dev
sudo apt-get install -y docutils-common
cd bluez-5.60
./configure --enable-library
make -j4
sudo make install
cd ~
git clone https://github.com/intel-iot-devkit/tinyb
cd tinyb
mkdir build
cd build
cmake ..
make -j4
sudo make install
sudo ldconfig 
cd ~
sudo apt-get install -y ninja-build libiio-dev libiio-dev libiio-utils libasound-dev
sudo apt-get install -y libboost1.62-all-dev
sudo ldconfig
sudo apt-get remove -y pulseaudio
sudo apt install -y libxml2 libxml2-dev bison flex cmake git libaio-dev libboost-all-dev
git clone https://github.com/analogdevicesinc/libad9361-iio
sudo apt-get install -y doxygen
sudo apt-get install -y libiio-dev
sudo apt-get install -y libiio-utils
cd libad9361-iio
cmake ./CMakeLists.txt
mkdir build
make -j4
sudo make install
sudo ldconfig
cd ~
git clone https://github.com/pothosware/SoapyPlutoSDR
cd SoapyPlutoSDR
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
cd ~
#fi
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/sdrberry_settings_pluto.cfg
mv sdrberry_settings_pluto.cfg sdrberry_settings.cfg
git clone https://github.com/paulh002/sdrberry
cd sdrberry
mkdir build
cd build
cmake ..
make
sudo make install
cd ~
sudo sed -i 's/$/ vt.global_cursor_default=0/' /boot/cmdline.txt
sudo sed -i '/dtparam=i2c_arm=on/s/^#//g' /boot/config.txt
sudo reboot