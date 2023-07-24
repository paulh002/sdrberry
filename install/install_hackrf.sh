#!/bin/bash
#if false; then
sudo apt update
sudo apt install build-essential git cmake swig -y
sudo apt-get -y install binutils-dev 
sudo apt-get -y install libdw-dev
sudo apt-get -y install git
sudo apt-get -y install gfortran
sudo apt-get install -y\
    cmake g++ \
    libpython-dev python-numpy swig
sudo apt -y install hackrf
sudo apt -y install libhackrf-dev
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
git clone https://github.com/pothosware/SoapyHackRF
cd SoapyHackRF
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
cd ~
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
sudo apt-get install -y ninja-build libiio-dev libiio-dev libiio-utils libasound-dev
sudo apt-get install -y libboost-all-dev
sudo ldconfig
sudo apt-get remove -y pulseaudio
#fi
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/sdrberry_settings_hackrf.cfg
mv sdrberry_settings_hackrf.cfg sdrberry_settings.cfg
git clone https://github.com/paulh002/sdrberry
cd sdrberry
git checkout Dev
mkdir build
cd build
cmake ..
make
sudo make install
cd ~
sudo sed -i 's/$/ vt.global_cursor_default=0/' /boot/cmdline.txt
sudo sed -i '/dtparam=i2c_arm=on/s/^#//g' /boot/config.txt
sudo reboot