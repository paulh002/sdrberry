#!/bin/bash
sudo apt install -y autoconf automake libtool pkg-config
git clone https://github.com/Hamlib/Hamlib
cd Hamlib
git checkout 4.7.0
sed -i.bak 's/\.port_type[[:space:]]*=[[:space:]]*RIG_PORT_SERIAL,/\.port_type =          RIG_PORT_NETWORK,/' rigs/yaesu/ft891.c
sed -i.bak 's/\.timeout[[:space:]]*=[[:space:]]*2000,/.timeout =            300,/' rigs/yaesu/ft891.c
sed -i.bak -e '/\.serial_rate_min[[:space:]]*=/d' \
           -e '/\.serial_rate_max[[:space:]]*=/d' \
           -e '/\.serial_data_bits[[:space:]]*=/d' \
           -e '/\.serial_stop_bits[[:space:]]*=/d' \
           -e '/\.serial_parity[[:space:]]*=/d' \
           -e '/\.serial_handshake[[:space:]]*=/d' \
           rigs/yaesu/ft891.c
autoreconf -fi
./configure
make
sudo make install
sudo ldconfig
cd ~
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
cmake ..
make -j4
sudo make install
cd ~
cp ~/sdrberry/install/sdrberry.desktop ~/.local/share/applications/sdrberry.desktop
cp ~/sdrberry/install/js8call.desktop ~/.local/share/applications/js8call.desktop
sudo chmod +x ~/.local/share/applications/sdrberry.desktop
update-desktop-database ~/.local/share/applications
