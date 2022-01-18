#!/bin/bash
#if false; then
sudo apt update
sudo apt install build-essential git cmake swig -y
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
git clone git://github.com/jgaeddert/liquid-dsp.git
sudo apt-get install -y automake autoconf
cd liquid-dsp
./bootstrap.sh
./configure
make -j4
sudo make install
sudo ldconfig
cd ~
wget http://www.kernel.org/pub/linux/bluetooth/bluez-5.60.tar.xz
tar xvf bluez-5.60.tar.xz
sudo apt-get install -y libusb-dev libreadline-dev libglib2.0-dev libudev-dev libdbus-1-dev libical-dev
sudo apt-get install -y python-docutils
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
git clone https://github.com/thestk/rtaudio
cd rtaudio
git checkout 5.2.0
mkdir build
cd build
cmake ..
make -j4
sudo make install
sudo ldconfig 
cd~
sudo apt-get remove -y pulseaudio
cat ~/.asoundrc
cat << EOF | tee ~/.asoundrc

pcm.!default {
        type hw
        card 1
}

ctl.!default {
        type hw
        card 1
}
EOF
sudo sed -i "s/^defaults.ctl.card.*/defaults.ctl.card 1/" /usr/share/alsa/alsa.conf
sudo sed -i "s/^defaults.pcm.card.*/defaults.pcm.card 1/" /usr/share/alsa/alsa.conf
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
git clone https://github.com/paulh002/SoapyRadioberry
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
sudo cp ./libSoapyRadioberrySDR.so /usr/local/lib/SoapySDR/modules0.8/.
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
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/sdrberry_settings.cfg
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/sdrberry1.sh
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/executable/sdrberry
chmod +x sdrberry
chmod +x sdrberry1.sh
sudo sed 's/$/ vt.global_cursor_default=0/' /boot/cmdline.txt


