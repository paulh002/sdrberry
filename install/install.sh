#!/bin/bash
wrkdir='/home/pi'
usrdir=$HOME
OS=`uname -m`
bits=`getconf LONG_BIT`
MODEL=$(tr -d '\0' < /sys/firmware/devicetree/base/model)
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
if [ "$USER" = "pi" ]; then
    echo "Current user is pi."
else
    echo "Current user is not pi."
    echo "Installation user has to be pi"
	echo "============================================"
    exit
fi
echo "Detected model: $MODEL"
# Check for specific models
if [[ "$MODEL" == *"Raspberry Pi 4 Model B"* ]]; then
    echo "This is a Raspberry Pi 4 Model B."
	pimod='PI4'
elif [[ "$MODEL" == *"Raspberry Pi 3 Model B"* ]]; then
    echo "This is a Raspberry Pi 3 Model B."
	echo "Raspberry Pi 3 Model B. is not supported"
	exit
elif [[ "$MODEL" == *"Raspberry Pi 5"* ]]; then
    echo "This is a Raspberry Pi 5."
	pimod='PI5'
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
elif [[ $1 = "RTLD" ]]; then sdrboard='RTLD'
elif [[ $1 = "No" ]]; then sdrboard='No'
else
   echo "SDR Unit being used Supported: hackfr = HRF / HifiBerry = HFB / Pluto = PLT / RadioBerry = RDB / SDRPlay SDP / RTLSDR RTL / RTLSDR direct sampling RTLD/ No = No device"
   echo "./install.sh Device"
   echo "If you want to use the build branch add build like ./install.sh RDB build"
   echo "If you want to compile from code use ./install.sh RDB"
   exit
fi

if [[ $1 = "SDP" ]]; then 
   echo "Please install SDRPlay API first press ctrl-C to stop this script"
   sleep 5
fi
if [[ $2 = "build" ]]; then BUILD='YES'
   echo "build branch"
else
   echo "main branch"
fi

no_passwd=2; #default
while true; do
read -p "Disable password for sudo answer 1 for disable or 2 not disable sudo password? " type
case $type in
	[1]* ) no_passwd=1; break;;
	[2]* ) no_passwd=2; break;;
	* ) echo "Please answer 1 for disable or 2 not disable sudo password";
esac
done

if [[ $no_passwd == 1 ]];	then
	echo "pi ALL=(ALL) NOPASSWD: ALL" >> 010_pi-nopasswd
	sudo cp  010_pi-nopasswd /etc/sudoers.d/.
	sudo chmod 0440 /etc/sudoers.d/010_pi-nopasswd
	sudo usermod -a -G lpadmin pi
fi

Display=1; #default
while true; do
read -p "Type of display DSI-1 1 for HDMI-A-1 2 for HDMI-A-2 3? " type
case $type in
	[1]* ) Display=1; break;;
	[2]* ) Display=2; break;;
	* ) echo "Default is DSI-1 ";
esac
done

sudo apt update
sudo apt install -y build-essential git cmake g++ libpython3-dev python3-numpy swig cmake \
binutils-dev libdw-dev gfortran g++ swig hackrf libhackrf-dev libfftw3-dev \
ninja-build libiio-dev libiio-dev libiio-utils libasound-dev \
libboost-all-dev python3 libfftw3-dev python3-yaml libtinyxml2-dev \
libglfw3-dev vim libxkbcommon-dev libusb-1.0-0-dev libxml2-dev flex bison libavahi-client-dev libaio-dev libcurl4-openssl-dev foot
echo "set mouse-=a" >> ~/.vimrc
sudo ldconfig

echo "pi ALL=(ALL) NOPASSWD: ALL" >> 010_pi-nopasswd
sudo cp  010_pi-nopasswd /etc/sudoers.d/.
sudo chmod 0440 /etc/sudoers.d/010_pi-nopasswd

cd $wrkdir || exit
git clone https://github.com/pothosware/SoapySDR.git
cd SoapySDR
git pull origin master
mkdir build
cd build
cmake ..
make -j4
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
if [[ $BUILD == 'YES' ]]; then
git switch build
fi
mkdir build
cd build || exit
cmake ..
make -j4
make sdrweb
sudo make install
cd $wrkdir || exit

cp ~/sdrberry/install/sdrberry.desktop ~/.local/share/applications/sdrberry.desktop
sudo chmod +x ~/.local/share/applications/sdrberry.desktop
update-desktop-database ~/.local/share/applications

#build sdrberry
git clone https://github.com/paulh002/build_sdrberry
cd build_sdrberry || exit
mkdir build
cd build && cmake .. && make -j4
sudo make install
cd $wrkdir || exit

#cd to work dir . If does not exist exit script
cd $wrkdir || exit

if [[ $sdrboard == HRF ]] ; then
git clone https://github.com/pothosware/SoapyHackRF
cd SoapyHackRF || exit
mkdir build
cd build || exit
cmake ..
make
sudo make install
sudo ldconfig
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
git clone https://github.com/analogdevicesinc/libad9361-iio
cd libad9361-iio || exit
cmake ./CMakeLists.txt
mkdir build
make -j4
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

if [[ $sdrboard == RTLD ]] ; then
sudo apt -y install rtl-sdr librtlsdr-dev
git clone https://github.com/paulh002/SoapyRTLSDR.git
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

git clone  --depth=1 https://github.com/paulh002/Radioberry-2.x
git checkout build

#-----------------------------------------------------------------------------

if [[ $fpgatype == 1 ]]; then
	echo "Installing Radioberry gateware Cyclone 10 CL016..."
		
if [[ $pimod == PI5 ]];	then
	cd Radioberry-2.x/SBC/rpi-4/releases/dev/CL016
else
	cd Radioberry-2.x/SBC/rpi-4/releases/dev/CL016
fi
sudo cp ./radioberry.rbf /lib/firmware
cd ../../../../../..
	
echo ""
echo "Radioberry gateware Cyclone 10 CL016 installed."

fi

if [[ $fpgatype == 2 ]]; then
	echo "Installing Radioberry gateware Cyclone 10 CL025..."
	
if [[ $pimod == PI5 ]];	then
	cd Radioberry-2.x/SBC/rpi-4/releases/dev/CL025
else
	cd Radioberry-2.x/SBC/rpi-4/releases/dev/CL025
fi
sudo cp ./radioberry.rbf /lib/firmware
cd ../../../../../..
	
echo ""
echo "Radioberry gateware Cyclone 10 CL025 installed."
	
fi

#-----------------------------------------------------------------------------
echo "Installing Radioberry driver..."
#cd to work dir . If does not exist exit script
cd $wrkdir || exit

#unregister radioberry driver
sudo modprobe -r radioberry
	
if [ ! -d "/lib/modules/$(uname -r)/kernel/drivers/sdr" ]; then
	sudo mkdir /lib/modules/$(uname -r)/kernel/drivers/sdr
fi

if [[ $pimod == PI5 ]];	then
	cd Radioberry-2.x/SBC/rpi-5/archive/gpio-mode/driver
else
	cd Radioberry-2.x/SBC/rpi-4/device_driver/driver
fi

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
#sudo sed -i '1i\SUBSYSTEM=="radioberry", GROUP="radioberry", MODE="0660"' /etc/udev/rules.d/99-com.rules
sudo tee /etc/udev/rules.d/99-radioberry.rules > /dev/null <<EOF
SUBSYSTEM=="radioberry", MODE="0660", GROUP="radioberry"
EOF
echo ""
echo "Radioberry driver installed."

#-----------------------------------------------------------------------------
fi

#cd to work dir . If does not exist exit script
cd $wrkdir || exit

#Remove if not planning to use bluetooth.
#sudo apt-get remove -y pulseaudio

#wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/sdrberry_settings.cfg
#mv sdrberry_settings.cfg $usrdir/sdrberry_settings.cfg
cp ./sdrberry/install/sdrberry_settings.cfg $usrdir
if [[ $sdrboard == 'HRF' ]]; then
sed -i '/default = "radioberry"/c\default = "hackrf"' $usrdir/sdrberry_settings.cfg
elif [[ $sdrboard == 'HFB' ]]; then
sed -i '/default = "radioberry"/c\default = "hifiberry"' $usrdir/sdrberry_settings.cfg
elif [[ $sdrboard == 'PLT' ]]; then
sed -i '/default = "radioberry"/c\default = "plutosdr"' $usrdir/sdrberry_settings.cfg
elif [[ $sdrboard == 'RDB' ]]; then
sed -i '/default = "radioberry"/c\default = "radioberry"' $usrdir/sdrberry_settings.cfg
elif [[ $sdrboard == 'SDP' ]]; then
sed -i '/default = "radioberry"/c\default = "sdrplay"' $usrdir/sdrberry_settings.cfg
elif [[ $sdrboard == 'RTL' ]]; then
sed -i '/default = "radioberry"/c\default = "rtlsdr"' $usrdir/sdrberry_settings.cfg
fi
cp ./sdrberry/install/sdrstart.sh $usrdir
chmod +x $usrdir/sdrstart.sh
#cp ./sdrberry/install/crontab $usrdir
#crontab ./sdrberry/install/crontab
#cd to work dir . If does not exist exit script
cd $wrkdir || exit

#enable I2C
sudo sed -i 's/$/ vt.global_cursor_default=0 usbhid.mousepoll=2/' /boot/firmware/cmdline.txt
sudo sed -i '/dtparam=i2c_arm=on/s/^#//g' /boot/firmware/config.txt

#Configure Display
if [[ $Display == 2 ]];	then
	echo "configure HDMI-A-1"
	sed -i 's/Display[[:space:]]*=[[:space:]]*"DSI-1"/Display="HDMI-A-1"/g' $usrdir/sdrberry_settings.cfg
fi
if [[ $Display == 3 ]];	then
	echo "configure HDMI-A-2"
	sed -i 's/Display[[:space:]]*=[[:space:]]*"DSI-1"/Display="HDMI-A-2"/g' $usrdir/sdrberry_settings.cfg
fi

#Do Cleanup
#rm -rf sdrberry rtaudio liquid-dsp SoapyHifiBerry SoapyHackRF SoapySDR sdrberry_settings_*
sudo setcap cap_sys_boot+ep /usr/local/bin/sdrberry
# 4. Create the rule file
RULES_DIR="/etc/polkit-1/rules.d"
sudo mkdir -p "$RULES_DIR"
RULE_FILE="$RULES_DIR/49-sdrberry-reboot.rules"
sudo cat >  49-sdrberry-reboot.rules <<EOF
polkit.addRule(function(action, subject) {
    if (action.id == "org.freedesktop.login1.reboot" &&
        subject.user == "pi") {
        return polkit.Result.YES;
    }
});
EOF
sudo cp 49-sdrberry-reboot.rules $RULES_DIR/.
rm 49-sdrberry-reboot.rules
sudo chmod 644 "$RULE_FILE"
sudo chown root:root "$RULE_FILE"
while true; do
read -p "Reboot or stop and inspect install log: 1 = Reboot or 2 = Stop? " type
case $type in
	[1]* ) sudo reboot; break;;
	[2]* ) echo "Stop, please reboot before using sdrberry"; break;;
	* ) echo "Please answer 1 = reboot or 2 = stop script.";
esac
done
