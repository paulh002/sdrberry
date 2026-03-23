#!/bin/bash
#  execute line by line 

sudo apt install -y autoconf automake libtool pkg-config
git clone https://github.com/Hamlib/Hamlib

#in file ft891.c you can change the port type. 
#.port_type = RIG_PORT_NETWORK,

cd Hamlib
autoreconf -fi    # Generates configure from configure.ac
./configure
make
sudo make install
cd ~
#Js8call
sudo apt install -y libxcb-xinerama0
sudo apt install -y qt6-wayland
sudo apt install -y libxcb-xinerama0-dev
sudo apt install -y qt6-wayland-dev
sudo apt install -y qt6-multimedia-dev libqt6multimedia6 libqt6multimediawidgets6
sudo apt install -y qt6-serialport-dev libqt6serialport6
sudo apt install -y libudev-dev
git clone https://github.com/js8call/js8call
cd js8call
mkdir build
cd build
make -j4
sudo make install


rigctl -m 1036 -r 127.0.0.1:5000 f
//tx
pi@pi5a:~ $ rigctl -m 1036 -r 127.0.0.1:5000 T 1
pi@pi5a:~ $ rigctl -m 1036 -r 127.0.0.1:5000 T 0
