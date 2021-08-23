#!/bin/bash

sudo cp /tmp/VisualGDB/h/OneDrive/source/SoapyRadioberry/VisualGDB/Release/libSoapyRadioberrySDR.so /usr/local/lib/SoapySDR/modules0.8/.
cp /tmp/VisualGDB/h/OneDrive/source/sdrberry/VisualGDB/Release/sdrberry .
sudo chmod 666 /dev/radioberry
./sdrberry
