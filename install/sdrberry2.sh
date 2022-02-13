#!/bin/bash

sudo modprobe -r radioberry
sudo modprobe radioberry
#sudo cp /tmp/VisualGDB/c/Users/paulh/OneDrive/source/SoapyRadioberry/VisualGDB/Release/libSoapyRadioberrySDR.so /usr/local/lib/SoapySDR/modules0.8/.
#sudo cp /tmp/VisualGDB/c/Users/paulh/OneDrive/source/sdrberry/VisualGDB/Release/sdrberry /usr/local/bin/.
rsync -az pi@192.168.88.41:/tmp/VisualGDB/c/Users/paulh/OneDrive/source/sdrberry/VisualGDB/Release/sdrberry  sdrberryx
sudo cp sdrberryx /usr/local/bin/sdrberry 
sudo chmod 666 /dev/radioberry
sdrberry
