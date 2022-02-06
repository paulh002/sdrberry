#!/bin/bash

sudo cp /tmp/VisualGDB/c/Users/paulh/OneDrive/source/SoapyRadioberry/VisualGDB/Release/libSoapyRadioberrySDR.so /usr/local/lib/SoapySDR/modules0.8/.
sudo cp /tmp/VisualGDB/c/Users/paulh/OneDrive/source/sdrberry/VisualGDB/Release/sdrberry /usr/local/bin/.
sudo chmod 666 /dev/radioberry
sdrberry
