#!/bin/bash
sudo pkill sdrberry
sudo rm /tmp/sdrberry.lock
#sudo modprobe -r radioberry
#sudo modprobe radioberry
#sudo cp /tmp/VisualGDB/c/Users/paulh/OneDrive/source/SoapyRadioberry/VisualGDB/Release/libSoapyRadioberrySDR.so /usr/local/lib/arm-linux-gnueabihf/SoapySDR/modules0.8/.
#sudo cp /tmp/VisualGDB/c/Users/paulh/OneDrive/source/sdrberry/VisualGDB/Release/sdrberry /usr/local/bin/.
rsync -az pi@192.168.88.62:/home/pi/VisualGDB/c/Users/paulh/OneDrive/source/sdrberry/VisualGDB/Release/sdrberry  sdrberryx
sudo cp sdrberryx /usr/local/bin/sdrberry 
sudo setcap cap_sys_boot+ep /usr/local/bin/sdrberry
#sudo chmod 666 /dev/radioberry
#sudo sdrberry > /dev/null 2>&1 &
#sudo sdrberry
