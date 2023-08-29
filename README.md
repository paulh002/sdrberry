# sdrberry
Raspberry pi SDR tranceiver / fronted using lvgl gui 
!!! This is an ongoing project code only for information purpose only. 
The main goal for this project is to learn about c++ programming, liquid dsp, and for most the use of the GUI toolkit LVGL v8
Functionalit is in beta currently

This source code is still in development.  
Goal is to support Adalm pluto SDR, Radioberry, RTL-SDR, SDRPlay etc but it should also support other SDR receivers based on SoapySDR.
Also the goal is to support optical encoder and support I2C / serial interface for bandpass filtering
I use an ESP32 as CAT controler with a service for an optical encoder.
Alternative a Contour Shuttle express can be used as rotary controler and or a mouse. 

To install you need to configure rasberry pi OS in text only mode (64 bit Bullseye), create a sd card or even better a usb stick sd cards tend to fail after a while,
with 64 bit bullseye CLI mode. Configure wifi and use raspi-config to switch on I2C and boot to CLI only with logon. 
Compiled using VisualGDB or cmake, gcc and gfortran (for wsjtx_lib which includes the wsjtx fortran code of Joe Taylor)
copy the cfg file (in install directory) to the home dir

Remote control is possible with the use of framebuffer-vnc https://github.com/ponty/framebuffer-vncserver a config file is in the install directory.
Check the event number for the touch and keyboard device and adjust the device number like event1 accordingly

installation instruction in install_guide.txt
install script install.sh installs all components based on a fresh raspberry pi SD card (OS should be updated and in CLI mode)

## Hardware requirements
- Raspberry pi 4 Model B 
- LCD screen 5 or 7 inch 800x480 touchscreen using DSI connector
- It is prefered not to use a SD card but an USB stick like an Samsung Fit plus 32 GB or larger
- USB Audio adaptor for audio in and output

## This software makes use of opensource libraries like
- Noise reduction code is an port from DD4WH https://github.com/df8oe/UHSDR/wiki/Noise-reduction  adapted for raspberry pi
- WSJTX fortran library of Joe Taylor K1JT
- Liquid-dsp of Joseph D. Gaeddert
- Lvgl of Gabor Kiss-Vamosi
- and others

## ToDo
- Support for Midi contoler
- Cheap USB Hid controlers
- Optical encoder direct on GPIO (some hardware adaption needed) 
- Setup screen for Network and Wifi
- Different noise reduction schemas like lms etc.
- codec2 implementation (FreeDV)

## Done
- Adalm Pluto
- Rtlsdr support
- SDRPlay support
- Radioberry support
- Hackrf support
- QSD / QSE support with hifiberry board
- FM broadband (only receive)
- FM Narrowband
- SSB support
- Transmitter part
- USB CAT interface support (config serial controler in cfg file example ESP32 as CAT controler)
- i2c filter support for PCF8574
- USB Controler with ESP32 (Optical contoler for vfo and 2 mechanical encoders for volume and gain etc)
- Add audio device selection
- TFT brightness controll
- Support both Raspberry pi buster and Bullseye
- CMake compilation
- Morse code decoder
- Support for Contour Shuttle Express
- Receive, transmit FT8
- Noise reduction (spectral noise reduction, Kim)
- Support for audio sdr using Tayloe directsdr (Hifiberry)
- Mouse support
- Keyboard for FT8
- waterfall display

## Installation of libraries is necessary:
- Liquid DSP
- Alsa audio
- SoapySDR
- SoapyPlutoSDR (support for Adalm Pluto SDR)
- SoapySDRPlay (support for SDRPlay devices)
- SoapyRadioBerry (support radioberry)
- libiio (support for Adalm Pluto SDR)
- libad9361-iio (support for Adalm Pluto SDR)
- pthread

# Installation


## Install and compile with cmake
Download the install file in pi home directory (there are install files for different SDR's)
```
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/install.sh
chmod +x install.sh
./install.sh
```

## Install and compile with cmake for radioberry
Download the repository in pi home directory  
```
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/install-radioberry.sh
chmod +x install-radioberry.sh
./install-radioberry.sh
```
There are also scripts for hackrf and pluto available.

## Compile with cmake (assuming all libraries are available)
```
git clone https://github.com/paulh002/sdrberry  
cd sdrberry  
mkdir build
cmake ..  
make
sudo make install  
```
## start sdrberry  
```
first copy config file to pi home dir  
cd ~  
cp sdrberry/sdrberry_settings.cfg .  
sudo sdrberry  
```

## DSI display 5inch or 7inch  
In the directory "install/bullseye-7inch rpi" are example config.txt and cmdline.txt files for rotating the screen
5 inch DSI screens from waveshare do not need any adjustment. Sdrberry is currently tested for a resolution of 800 x 480. 
Although this can be changed in sdrberry.cpp line 47 and 48 it is not tested for other resolutions. There can be some issues.

![sdrberry](https://github.com/paulh002/sdrberry/blob/master/rb_tranceiver.jpg)

![sdrberry](https://github.com/paulh002/sdrberry/blob/master/IMG_20220203_ft8.jpg)

![sdrberry](https://github.com/paulh002/sdrberry/blob/master/IMG_20211215_200645.jpg)

![sdrberry](https://github.com/paulh002/sdrberry/blob/master/IMG_20210909_183113.jpg)

[![Radioberry demo](https://img.youtube.com/vi/BMJiv3YGv-k/0.jpg)](https://youtu.be/PQ_Np5SfcxA)

# ESP32 Remote control for raspberry pi
![sdrberry](https://github.com/paulh002/sdrberry/blob/master/IMG_20210903_133827.jpg)