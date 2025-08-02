![example event parameter](https://github.com/paulh002/sdrberry/actions/workflows/build-arm64.yml/badge.svg?branch=build)

# sdrberry
Raspberry pi SDR transceiver / frontend using lvgl gui 
!!! This is an ongoing project code only for information purpose only. 
The main goal for this project is to learn about c++ programming, liquid dsp, and for most the use of the GUI toolkit LVGL v8
Functionality is in beta currently

This source code is still in development.  
Goal is to support Adalm pluto SDR, Radioberry, RTL-SDR, SDRPlay etc but it should also support other SDR receivers based on SoapySDR.
Also the goal is to support optical encoder and support I2C / serial interface for bandpass filtering
I use an ESP32 as CAT controller with a service for an optical encoder.
Alternative a Contour Shuttle express can be used as rotary controller and or a mouse. 

To install you need to configure rasberry pi OS in text only mode (64 bit Bullseye), create a sd card or even better a usb stick sd cards tend to fail after a while,
with 64 bit bullseye CLI mode. Configure wifi and use raspi-config to switch on I2C and boot to CLI only with logon. 
Compiled using VisualGDB or cmake, gcc and gfortran (for wsjtx_lib which includes the wsjtx fortran code of Joe Taylor)

Remote control is possible with the use of framebuffer-vnc https://github.com/ponty/framebuffer-vncserver a config file is in the install directory.
Check the event number for the touch and keyboard device and adjust the device number like event1 accordingly

installation instruction in install_guide.txt
install script install.sh installs all components based on a fresh raspberry pi SD card (OS should be updated and in CLI mode)


## Hardware requirements
- Raspberry pi 4 Model B or Pi 5
- LCD screen 5 or 7 inch 800x480 touchscreen using DSI connector or the new raspberry Touch 2 7 inch display
- It is preferred not to use a SD card but an USB stick like an Samsung Fit plus 32 GB or larger
- Generic USB Audio adapter for audio in and output

## This software makes use of opensource libraries like
- Noise reduction code is an port from DD4WH https://github.com/df8oe/UHSDR/wiki/Noise-reduction  adapted for raspberry pi
- WSJTX fortran library of Joe Taylor K1JT
- Liquid-dsp of Joseph D. Gaeddert
- Lvgl of Gabor Kiss-Vamosi
- and others

## Under development
- webserver

## ToDo
- Support for Midi controller
- Optical encoder direct on GPIO (some hardware adaption needed) 
- Setup screen for Network and Wifi
- Different noise reduction schemes like lms etc.
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
- USB CAT interface support (config serial controller in cfg file example ESP32 as CAT controller)
- i2c filter support for PCF8574
- USB controller with ESP32 (Optical controller for vfo and 2 mechanical encoders for volume and gain etc)
- Add audio device selection
- TFT brightness control
- Support both Raspberry pi buster and Bullseye
- CMake compilation
- Morse code decoder
- Support for Contour Shuttle Express
- Receive, transmit FT8
- Noise reduction (spectral noise reduction, Kim)
- Support for audio sdr using Taylor directsdr (Hifiberry)
- Mouse support
- Keyboard for FT8
- waterfall display
- Cheap USB Hid Volume controllers as tuning controller
- Add setup page for I2C configuration
- Cheap USB Hid controllers
- Support for output only audio devices
- I2C RX/TX switch support
- CAT support for N1MM (tcp connection)
- WSJT-X UDP client support. Sdrberry will send Heartbeat, Decode and QSO messages to designated ip

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
- liquid-dsp
- fftw
- civetweb
- nlohmann json
- HowardHinnant date.h library

# Installation
## Preparation before install
Take a good quality USB drive and install the latest 64 bit Raspberry OS on it.
I advise not to use SD cards, these are very slow and often fail in a short time.
Use raspi-config to switch to console auto logon startup.
Update the OS with the latest raspberry pi OS patches

## Install and compile with cmake
Download the install file in pi home directory (the install script supports different SDR's)
The install script can install DSI (800x480 16bits color), Raspberry PI Touch 1 7inch 800x480 (32 bit color), Raspberry PI Touch 2 7inch 1280x720 ,  Waveshare 7 inch display
SDR Unit being used Supported: hackfr = HRF / HifiBerry = HFB / Pluto = PLT / RadioBerry = RDB / SDRPlay = SDP /RTLSDR RTL / No = No device


DSI is 16 bit color depth display (DSI / HDMI) the resolution is configurable in the setting file see the wiki. 
https://github.com/paulh002/sdrberry/wiki/Screen-resolution

```
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/install.sh
chmod +x install.sh
./install.sh HFB DSI
```
Raspberry PI Touch 1 with Radioberry (32 bit colordepth !)
```
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/install.sh
chmod +x install.sh
./install.sh RDB T1
```
Raspberry PI Touch 2 with Radioberry (32 bit colordepth !)
```
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/install.sh
chmod +x install.sh
./install.sh RDB T2
```
## start sdrberry  
sdrberry can be started in user mode or as root, this depends on the device being used for radioberry use sudo
```
sudo sdrberry > sdrberry.log 2>&1
```

## Mouse support
sdrberry supports a mouse, to improve responsiveness the install script will add usbhid.mousepoll=2 to the commandline.txt  


## DSI display 5inch or 7inch  
In the directory "install/bullseye-7inch rpi" are example config.txt and cmdline.txt files for rotating the screen
5 inch DSI screens from waveshare do not need any adjustment. Sdrberry is currently tested for a resolution of 800 x 480. 
Although this can be changed in sdrberry.cpp line 47 and 48 it is not tested for other resolutions. There can be some issues.

![sdrberry](https://github.com/paulh002/sdrberry/blob/0354b16771afa0fb7ccf55b9a41a7d6fa6422540/images/rb_tranceiver.jpg)

![sdrberry](https://github.com/paulh002/sdrberry/blob/0354b16771afa0fb7ccf55b9a41a7d6fa6422540/images/IMG_20220203_ft8.jpg)

[![Radioberry demo](https://i9.ytimg.com/vi_webp/ycU_IUmV8j4/mq1.webp?sqp=CMC2268G-oaymwEmCMACELQB8quKqQMa8AEB-AH-CYAC0AWKAgwIABABGBggSih_MA8=&rs=AOn4CLC4GjVI4ExJf2BNTO02C6DS9CYR2g)](https://youtu.be/ycU_IUmV8j4?si=ixo44dyJ-GQGvl-Y)

# I2C configuration
![sdrberry](https://github.com/paulh002/sdrberry/blob/0354b16771afa0fb7ccf55b9a41a7d6fa6422540/images/setup_it2c.jpg)

# WSJT-X & CAT integration for logging software
Sdrberry sends messages to Qlog (or other logging software like NN1MM)
And can be remote controled by logging software using CAT FT-981 commands.
![sdrberry](https://github.com/user-attachments/assets/923a954f-c64e-487f-994d-bfa7133193f7)

# ESP32 Remote control for raspberry pi
![sdrberry](images/IMG_20231015_131803.jpg)

# Web Server on port 8081
First version of sdrberry webserver, goal is to remote control the functions of sdrberry.
This is still an experiment / beta version. To see what is the best way to integrate web with C++ application.
WEB pages are build with VUE JS 3 and Primevue source code will be shared later in separate github repo.

Copy sdrweb directory to pi home dir 
Open the WSJTX webpage on http://raspberry_pi_ip:8081

![sdrberry](https://github.com/paulh002/sdrberry/blob/0354b16771afa0fb7ccf55b9a41a7d6fa6422540/images/sdrweb.png)
