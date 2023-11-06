#!/bin/bash
sudo pkill sdrberry
cd sdrberry
git pull
cd build
cmake ..
make -j4
sudo make install
cd ~

