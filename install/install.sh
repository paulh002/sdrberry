#!/bin/bash
wrkdir='/home/pi'
usrdir='/home/pi'

if [[ $1 = "HRF" ]]; then sdrboard='HRF'
elif [[ $1 = "HFB" ]]; then sdrboard='HFB'
elif [[ $1 = "PLT" ]]; then sdrboard='PLT'
elif [[ $1 = "RDB" ]]; then sdrboard='RDB'
else
   echo "SDR Unit being used Supported: hackfr = HRF / HifiBerry = HFB / Pluto = PLT / RadioBerry = RDB"
   echo "LCD devices supported are 800x480 DSI 7 inch and 5 inch, Waveshare 7 inch 1200x600 WC12"
   echo "install Device LCD PACKAGE  "
   echo  "./install.sh RDB DSI Y"
   exit
fi
if [[ $2 = "DSI" ]]; then LCD='DSI'
elif [[ $2 = "WC12" ]]; then LCD='7c'
else
   echo "SDR Unit being used Supported: hackfr = HRF / HifiBerry = HFB / Pluto = PLT / RadioBerry = RDB"
   echo "LCD devices supported are 800x480 DSI 7 inch and 5 inch, Waveshare 7 inch 1200x600 WC12"
   echo "install Device LCD PACKAGE  "
   echo  "./install.sh RDB DSI Y"
   exit
fi

if [[ $3 = "Y" ]]; then PACKAGES='YES'
else
   echo "No packages"
fi

#if false; then
sudo apt update
sudo apt install -y build-essential git cmake g++ libpython3-dev python3-numpy swig \
binutils-dev libdw-dev gfortran g++ swig hackrf libhackrf-dev libfftw3-dev \
ninja-build libiio-dev libiio-dev libiio-utils libasound-dev \
libboost-all-dev python3 libfftw3-dev \
libglfw3-dev vim
echo "set mouse-=a" >> ~/.vimrc
sudo ldconfig

cd $wrkdir || exit
git clone https://github.com/pothosware/SoapySDR.git
cd SoapySDR
git pull origin master
mkdir build
cd build
cmake ..
make
sudo make install
sudo ldconfig
cd $wrkdir || exit

#build Liquid-dsp
git clone https://github.com/jgaeddert/liquid-dsp
sudo apt-get install -y automake autoconf
cd liquid-dsp || exit
git checkout v1.6.0
./bootstrap.sh
./configure
make -j4
sudo make install
sudo ldconfig

#cd to work dir . If does not exist exit script
cd $wrkdir || exit

#build sdrberry
git clone https://github.com/paulh002/sdrberry
cd sdrberry || exit
git checkout installation-improvement
mkdir build
cd build && cmake .. && make -j4
sudo make install

#cd to work dir . If does not exist exit script
cd $wrkdir || exit

if [[ $sdrboard == HRF ]] ; then
if [[ $PACKAGES == 'YES' ]]; then
sudo apt install soapysdr0.8-module-hackrf
else
git clone https://github.com/pothosware/SoapyHackRF
cd SoapyHackRF || exit
mkdir build
cd build || exit
cmake ..
make
sudo make install
sudo ldconfig
fi
fi

#cd to work dir . If does not exist exit script
cd $wrkdir || exit

if [[ $sdrboard == HFB ]] ; then
git clone https://github.com/paulh002/SoapyHifiBerry
cd SoapyHifiBerry || exit
mkdir build
cd build || exit
cmake ..
make
sudo make install
sudo ldconfig
fi

if [[ $sdrboard == PLT ]] ; then
git clone https://github.com/pothosware/SoapyPlutoSDR
cd SoapyPlutoSDR || exit
mkdir build
cd build || exit
cmake ..
make
sudo make install
sudo ldconfig
fi

if [[ $sdrboard == RDB ]] ; then
#-----------------------------------------------------------------------------
echo "Installing Radioberry driver..."
git clone --depth=1 https://github.com/pa3gsb/Radioberry-2.x

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
cd SoapyRadioberry/driver || exit
make

sudo cp radioberry.ko /lib/modules/$(uname -r)/kernel/drivers/sdr
sudo dtc -@ -I dts -O dtb -o radioberry.dtbo radioberry.dts
sudo cp radioberry.dtbo /boot/overlays
#add driver to config.txt
sudo grep -Fxq "dtoverlay=radioberry" /boot/config.txt || sudo sed -i '$ a dtoverlay=radioberry' /boot/config.txt
sudo cp ./radioberry.rbf /lib/firmware/.

#cd to work dir . If does not exist exit script 
cd  $wrkdir || exit

sudo depmod	
#register radioberry driver
sudo modprobe radioberry
sudo chmod 666 /dev/radioberry
#show radioberry driver info.
sudo modinfo radioberry

echo ""
echo "Radioberry driver installed."
#-----------------------------------------------------------------------------
fi

#cd to work dir . If does not exist exit script
cd $wrkdir || exit

if [[ $sdrboard == PLT ]] ; then
if [[ $PACKAGES == 'YES' ]]; then
sudo apt install libad9361-dev libad9361-0 python3-ad9361
else
git clone https://github.com/analogdevicesinc/libad9361-iio
cd libad9361-iio || exit
cmake ./CMakeLists.txt
mkdir build
make -j4
sudo make install
sudo ldconfig
fi
fi

#cd to work dir . If does not exist exit script
cd $wrkdir || exit

#Remove if not planning to use bluetooth.
sudo apt-get remove -y pulseaudio

if [[ $sdrboard == 'HRF' ]]; then
# put sdrberry config in users home dir
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/sdrberry_settings_hackrf.cfg
mv sdrberry_settings_hackrf.cfg $usrdir/sdrberry_settings.cfg
elif [[ $sdrboard == 'HFB' ]]; then
# put sdrberry config in users home dir
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/sdrberry_settings.cfg
mv sdrberry_settings.cfg $usrdir/sdrberry_settings.cfg
elif [[ $sdrboard == 'PLT' ]]; then
# put sdrberry config in users home dir
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/sdrberry_settings_pluto.cfg
mv sdrberry_settings_pluto.cfg $usrdir/sdrberry_settings.cfg
elif [[ $sdrboard == 'RDB' ]]; then
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/sdrberry_settings_radioberry.cfg
mv sdrberry_settings_radioberry.cfg $usrdir/sdrberry_settings.cfg
fi

cp ./sdrberry/install/sdrstart.sh $usrdir
chmod +x $usrdir/sdrstart.sh

#Do LCD Screen Setup
#if [[ $LCD == '5' || $LCD == '7' || $LCD == '7b' ]]; then
#Enable Waveshare 7inch_DSI_LCD_(5/7/7B) 800x480
#echo dtoverlay=vc4-kms-v3d >> /boot/config.txt
#echo dtoverlay=vc4-kms-dsi-7inch >> /boot/config.txt
#
#fi

if [[ $LCD == '7c' ]]; then
#Enable Waveshare 7inch DSI LCD (C) 1024×600 driver：
git clone https://github.com/waveshare/Waveshare-DSI-LCD
cd Waveshare-DSI-LCD/6.1.21/64 || exit
sudo bash ./WS_xinchDSI_MAIN.sh 70C I2C1
fi

#cd to work dir . If does not exist exit script
cd $wrkdir || exit

#enable I2C
sudo sed -i 's/$/ vt.global_cursor_default=0 usbhid.mousepoll=2/' /boot/cmdline.txt
sudo sed -i '/dtparam=i2c_arm=on/s/^#//g' /boot/config.txt

#Do Cleanup
#rm -rf sdrberry rtaudio liquid-dsp SoapyHifiBerry SoapyHackRF SoapySDR sdrberry_settings_*

sudo reboot