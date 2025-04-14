#!/bin/bash
wrkdir='/home/pi'
usrdir=$HOME
OS=`uname -m`
bits=`getconf LONG_BIT`
echo ""
echo ""
echo "============================================"
echo "sdrberry software installation."
echo ""
if [ $OS = "aarch64" ]; then
        echo "	64 bit kernel"
fi
if [ $bits = "64" ]; then
        echo "	64 bit OS"
fi
if [ $bits = "32" ]; then
        echo "	32 bit OS"
fi
if [ $bits = "32" ] && [ $OS = "aarch64" ]; then
        echo "	64 kernel and 32 bit OS not supported"
		echo "============================================"
        exit
fi
echo "============================================"
echo ""
echo ""
if [[ $1 = "HRF" ]]; then sdrboard='HRF'
elif [[ $1 = "HFB" ]]; then sdrboard='HFB'
elif [[ $1 = "PLT" ]]; then sdrboard='PLT'
elif [[ $1 = "RDB" ]]; then sdrboard='RDB'
elif [[ $1 = "SDP" ]]; then sdrboard='SDP'
elif [[ $1 = "RTL" ]]; then sdrboard='RTL'
elif [[ $1 = "No" ]]; then sdrboard='No'
else
   echo "SDR Unit being used Supported: hackfr = HRF / HifiBerry = HFB / Pluto = PLT / RadioBerry = RDB / SDRPlay SDP / RTLSDR RTL / No = No device"
   echo "./install_extra_sdr.sh Device"
   exit
fi
if [[ $1 = "SDP" ]]; then 
   echo "Please install SDRPlay API first press ctrl-C to stop this script"
   sleep 5
fi

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
sudo sed -i '$a\dtoverlay=hifiberry-dacplusadcpro' /boot/config.txt
fi

#cd to work dir . If does not exist exit script
cd $wrkdir || exit

if [[ $sdrboard == PLT ]] ; then
if [[ $PACKAGES == 'YES' ]]; then
sudo apt install -y libad9361-dev libad9361-0
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

if [[ $sdrboard == RTL ]] ; then
sudo apt -y install rtl-sdr librtlsdr-dev
git clone https://github.com/pothosware/SoapyRTLSDR.git
cd SoapyRTLSDR || exit
mkdir build
cd build || exit
cmake ..
make
sudo make install
sudo ldconfig
fi

if [[ $sdrboard == RDB ]] ; then
git clone https://github.com/paulh002/SoapyRadioberry
cd SoapyRadioberry || exit
mkdir build
cd build || exit
cmake ..
make
sudo make install
sudo ldconfig
fi

if [[ $sdrboard == SDP ]] ; then
git clone https://github.com/pothosware/SoapySDRPlay3
cd SoapySDRPlay3 || exit
mkdir build
cd build || exit
cmake ..
make
sudo make install
sudo ldconfig
fi

#cd to work dir . If does not exist exit script
cd $wrkdir || exit

if [[ $sdrboard == RDB ]] ; then
#-----------------------------------------------------------------------------
fpgatype=2; #default
while true; do
read -p "Install CL016 or CL025 radioberry version: 1 = CL016 or 2 = CL025? " type
case $type in
	[1]* ) fpgatype=1; break;;
	[2]* ) fpgatype=2; break;;
	* ) echo "Please answer 1 or 2 for the FPGA used in your radioberry.";
esac
done

function install_dependency {
	echo "--- Installing dependency: $1"
	sudo apt-get -y install $1
}

install_dependency raspberrypi-kernel-headers
install_dependency git
install_dependency device-tree-compiler
install_dependency pigpio

git clone  --depth=1 https://github.com/paulh002/Radioberry-2.x
git checkout build

#-----------------------------------------------------------------------------

if [[ $fpgatype == 1 ]]; then
	echo "Installing Radioberry gateware Cyclone 10 CL016..."
		
cd Radioberry-2.x/SBC/rpi-4/releases/dev/CL016
sudo cp ./radioberry.rbf /lib/firmware
cd ../../../../../..
	
echo ""
echo "Radioberry gateware Cyclone 10 CL016 installed."

fi

if [[ $fpgatype == 2 ]]; then
	echo "Installing Radioberry gateware Cyclone 10 CL025..."
	
cd Radioberry-2.x/SBC/rpi-4/releases/dev/CL025
sudo cp ./radioberry.rbf /lib/firmware
cd ../../../../../..
	
echo ""
echo "Radioberry gateware Cyclone 10 CL025 installed."
	
fi

#-----------------------------------------------------------------------------
echo "Installing Radioberry driver..."

#unregister radioberry driver
sudo modprobe -r radioberry
	
if [ ! -d "/lib/modules/$(uname -r)/kernel/drivers/sdr" ]; then
	sudo mkdir /lib/modules/$(uname -r)/kernel/drivers/sdr
fi
	
cd Radioberry-2.x/SBC/rpi-4/device_driver/driver
make
sudo cp radioberry.ko /lib/modules/$(uname -r)/kernel/drivers/sdr

sudo dtc -@ -I dts -O dtb -o radioberry.dtbo radioberry.dts
sudo cp radioberry.dtbo /boot/overlays
#add driver to config.txt
sudo grep -Fxq "dtoverlay=radioberry" /boot/firmware/config.txt || sudo sed -i '$ a dtoverlay=radioberry' /boot/firmware/config.txt

cd ../../../../..

sudo depmod	
#register radioberry driver
sudo modprobe radioberry
sudo chmod 666 /dev/radioberry
#show radioberry driver info.
sudo modinfo radioberry
sudo groupadd radioberry
sudo usermod -aG radioberry pi
sudo sed -i '1i\SUBSYSTEM=="radioberry", GROUP="radioberry", MODE="0660"' /etc/udev/rules.d/99-com.rules
echo ""
echo "Radioberry driver installed."

#-----------------------------------------------------------------------------
fi

#cd to work dir . If does not exist exit script
cd $wrkdir || exit