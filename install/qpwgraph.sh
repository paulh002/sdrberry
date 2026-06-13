sudo apt update
sudo apt install -y libxcb-xinerama0
sudo apt install -y qt6-wayland
sudo apt install -y libxcb-xinerama0-dev
sudo apt install -y qt6-wayland-dev
sudo apt install -y qt6-multimedia-dev libqt6multimedia6 libqt6multimediawidgets6
sudo apt install -y qt6-serialport-dev libqt6serialport6
sudo apt install -y libudev-dev
sudo apt install libpipewire-0.3-dev
sudo apt install qt6-svg-dev qt6-svg-plugins
git clone https://github.com/rncbc/qpwgraph
cd qpwgraph
cmake -B build
cmake --build build -j4