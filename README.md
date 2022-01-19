# sdrberry
Raspberry pi SDR receiver/ fronted using lvgl gui
!!! This is an ongoing project code only for information purpose only. 
The main goal for this project is to learn about c++ programming, liquid dsp, and for most the use of the GUI toolkit LVGL v8
Functionalit is in beta currently

This source code is still in development.  
Goal is to support Adalm pluto SDR, Radioberry, RTL-SDR, SDRPlay etc but it should also support other SDR receivers based on SoapySDR.
Also the goal is to support optical encoder and support I2C / serial interface for bandpass filtering
Currently it also supports basic BLE as controler input. I use a ESP32 as BLE server with a service for an optical encoder.

To install you need to configure rasberry pi OS in text only mode, (switch off the GUI)
Compiled using VisualGDB or cmake and gcc
copy the cfg file (in install directory) to the home dir

a compiled executable and soapyradioberry library is available in executable directory
installation instruction in install_guide.txt
install script install.sh installs all components based on a fresh raspberry pi SD card (OS should be updated and in CLI mode)

ToDo:
- Support for Midi contoler
- Setup screen for Network and Wifi
- waterfall display
- Optimze resampling to keep delay in audio to minimum

Done:
- Adalm Pluto
- Rtlsdr support
- SDRPlay support
- Radioberry support
- FM broadband (only receive)
- FM Narrowband
- SSB support
- Basic BLE support
- Transmitter part
- USB CAT interface support (config serial controler in cfg file example ESP32 as CAT controler)
- i2c filter support for PCF8574
- USB Controler with ESP32 (Optical contoler for vfo and 2 mechanical encoders for volume and gain etc)
- Add audio device selection
- TFT brightness controll
- Support both Raspberry pi buster and Bullseye
- CMake compilation

Installation of libraries is necessary:
- Liquid DSP
- Alsa audio
- SoapySDR
- SoapyPlutoSDR (support for Adalm Pluto SDR)
- SoapySDRPlay (support for SDRPlay devices)
- SoapyRadioBerry (support radioberry)
- libiio (support for Adalm Pluto SDR)
- libad9361-iio (support for Adalm Pluto SDR)
- bluez-5.60
- tinyb
- pthread

# Installation


## Install and compile with cmake
Download the repository in pi home directory  
```
git clone https://github.com/paulh002/sdrberry  
chmod +x ./sdrberry/install/install.sh
./sdrberry/install/install.sh
```
## Compile with cmake (assuming all libraries are available)
```
git clone https://github.com/paulh002/sdrberry  
cd sdrberry  
mkdir build  
chmod +x ../do_cmake.sh  
../do_cmake.sh  
sudo make install  
```
## start sdrberry  
```
first copy config file to pi home dir  
cd ~  
cp sdrberry/sdrberry_settings.cfg .  
sudo sdrberry  
```

## DSI display 7inch  
In the directory "install/bullseye-7inch rpi" are example config.txt and cmdline.txt files for rotating the screen
5 inch DSI screens from waveshare do not need any adjustment

![sdrberry](https://github.com/paulh002/sdrberry/blob/master/IMG_20211215_200645.jpg)

![sdrberry](https://github.com/paulh002/sdrberry/blob/master/IMG_20210909_183113.jpg)

[![Radioberry demo](https://img.youtube.com/vi/BMJiv3YGv-k/0.jpg)](https://youtu.be/PQ_Np5SfcxA)

# ESP32 Remote control for raspberry pi
![sdrberry](https://github.com/paulh002/sdrberry/blob/master/IMG_20210903_133827.jpg)