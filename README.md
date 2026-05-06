![example event parameter](https://github.com/paulh002/sdrberry/actions/workflows/build-arm64.yml/badge.svg?branch=build)

# Sdrberry wayland
Raspberry pi SDR transceiver / frontend using lvgl gui supporting Raspberry pi OS Wayland.
!!! This is an ongoing project code only for information purpose only. 
The main goal for this project is to learn about c++ programming, liquid dsp, and for most the use of the GUI toolkit LVGL v9
Functionality is in beta currently

This source code is still in development.  
Goal is to support Adalm pluto SDR, Radioberry, RTL-SDR, SDRPlay etc but it should also support other SDR receivers based on SoapySDR.
Also the goal is to support optical encoder and support I2C / serial interface for bandpass filtering
I use an ESP32 as CAT controller with a service for an optical encoder.
Alternative a Contour Shuttle express can be used as rotary controller and or a mouse. 

install script install.sh installs all components based on a fresh raspberry pi USB drive or SD card (OS should be updated)

## Licence
This software is licensed under GPLv3. Commercial hardware manufacturers are welcome to bundle Sdrberry, 
provided they comply with the GPLv3 terms (i.e., provide full source code and preserve user freedoms).

## Hardware requirements
- Raspberry pi 4 Model B or Pi 5
- LCD screen 5 or 7 inch 800x480 touchscreen using DSI connector or the new raspberry Touch 2 7 inch 1280 x 720 display
- It is preferred not to use a SD card but an USB stick like an Samsung Fit plus 32 GB or larger
- Generic USB Audio adapter for audio in and output
- Optional I2C GPIO for BPF, LPF and RX/TX switching
- Advised to use a USB drive instead of an SD card.

## OS requirements
- Raspberry pi OS Trixie or newer

## This software makes use of opensource libraries like
- Noise reduction code is an port from DD4WH https://github.com/df8oe/UHSDR/wiki/Noise-reduction  adapted for raspberry pi
- WSJTX fortran library of Joe Taylor K1JT
- Liquid-dsp of Joseph D. Gaeddert
- Lvgl of Gabor Kiss-Vamosi
- and others

## Under development
- WSPR transmit
- webserver

## ToDo
- Support for Midi controller
- Optical encoder direct on I2C (some I2C GPIO adaption needed) 
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
- Shutdown button for save shutdown of the pi
- Wayland support
- WSPR (receiving)

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
- wsjtx_lib

# Installation
## Preparation before install
Take a good quality USB drive and install the latest 64 bit Raspberry OS on it.
I advise not to use SD cards, these are very slow and often fail in a short time.
Use raspi-config to switch to GUI auto logon startup.
Update the OS with the latest raspberry pi OS patches

# Pi 5 considerations
This wayland version works both on Pi4 and Pi5 Trixie Raspberry pi OS
Older distributions are not tested and not supported.

# Pi 5 and Radioberry considerations
To use the pi5 and the radioberry there are two options for the radioberry driver. 
1) Use the GPIO version of the driver. This version will work directly on Raspberry pi OS but has some performace limitations.
Use this version only with lower samplerates like 96 Khz or lower.
2) Use the PIO driver. This driver uses DMA to transfer the data from the Radioberry to the pi and is very efficient.
It has no samplerate limitation, only requires a patched Linux kernel. Supplied by PA3GSB or you can patch the kernel your self.

## Install and compile with cmake
Download the install file in pi home directory (the install script supports different SDR's)
SDR Unit being used Supported: hackfr = HRF / HifiBerry = HFB / Pluto = PLT / RadioBerry = RDB / SDRPlay = SDP /RTLSDR RTL / No = No device
```
wget https://raw.githubusercontent.com/paulh002/sdrberry/master/install/install.sh
chmod +x install.sh
./install.sh RDB
```
## create menu short cut
copy sdrberry.desktop to ~/.local/share/applications
```
cp ~/sdrberry/install/sdrberry.desktop ~/.local/share/applications/sdrberry.desktop
sudo chmod +x ~/.local/share/applications/sdrberry.desktop
update-desktop-database ~/.local/share/applications
```
## Raspberry pi Touch 2 7 or 5 inch desktop rotation
To rotate the Raspberry Touch 2 displays permenent execute folowing script.
```
chmod +x ./sdrberry/install/desktop-rotation.sh
./sdrberry/install/desktop-rotation.sh
```

## autostart
To automaticly start sdrberry edit:
```
vi ./config/labwc/autostart
sdrberry >> /home/pi/sdrberry.log 2>&1
```
Or combine with screen rotation
```
vi ./config/labwc/autostart
wlr-randr --output DSI-1 --transform 90 &
sdrberry >> /home/pi/sdrberry.log 2>&1
```

## Slow Mouse configuration
Wayland supports a mouse, to improve responsiveness the install script will add usbhid.mousepoll=2 to the commandline.txt  

## start sdrberry  
From the Accessories menu choose sdrberry to start the software.
Or start from cli in that case make sure the WAYLAND_DISPLAY environment variable is set.

```
export WAYLAND_DISPLAY=wayland-0
sdrberry > sdrberry.log 2>&1
```
![Image](https://github.com/user-attachments/assets/256f1ea1-a3b0-4959-964e-2534f06dc5ac)

## WSPR support
Sdrberry can receive and decode wspr messages
![Image](https://github.com/user-attachments/assets/a89b8f7f-e389-44f0-a8a7-c3cfbc976d02)

## Integration with JS8CALL
Sdrberry can communicate with js8call using pipewire on the same pi
To configure pipewire execute the setup-pipewire.sh
Sdrberry emulated the CAT interface of an yeasu ft891 using tcp/ip. For js8call to connect to the radioberry we need to patch hamlib.
This is automaticly done using the install_js8.sh script.

```
./setup_pipewire.sh
./install_js8.sh
```
![Image](https://github.com/user-attachments/assets/60c3ad81-2a99-4665-83f5-dd23a6b4171e)

## Tranceiver with sdrberry and radioberry
![sdrberry](https://github.com/paulh002/sdrberry/blob/0354b16771afa0fb7ccf55b9a41a7d6fa6422540/images/rb_tranceiver.jpg)

![sdrberry](https://github.com/paulh002/sdrberry/blob/0354b16771afa0fb7ccf55b9a41a7d6fa6422540/images/IMG_20220203_ft8.jpg)

[![Radioberry demo](https://i9.ytimg.com/vi_webp/ycU_IUmV8j4/mq1.webp?sqp=CMC2268G-oaymwEmCMACELQB8quKqQMa8AEB-AH-CYAC0AWKAgwIABABGBggSih_MA8=&rs=AOn4CLC4GjVI4ExJf2BNTO02C6DS9CYR2g)](https://youtu.be/ycU_IUmV8j4?si=ixo44dyJ-GQGvl-Y)

# I2C configuration
![sdrberry](https://github.com/paulh002/sdrberry/blob/0354b16771afa0fb7ccf55b9a41a7d6fa6422540/images/setup_it2c.jpg)

# WSJT-X & CAT integration for logging software
Sdrberry sends messages to Qlog (or other logging software like NN1MM)
And can be remote controled by logging software using CAT FT-981 commands.
![sdrberry](https://github.com/user-attachments/assets/923a954f-c64e-487f-994d-bfa7133193f7)

# Web Server on port 8081
First version of sdrberry webserver, goal is to remote control the functions of sdrberry.
This is still an experiment / beta version. To see what is the best way to integrate web with C++ application.
WEB pages are build with VUE JS 3 and Primevue source code will be shared later in separate github repo.

Copy sdrweb directory to pi home dir 
Open the WSJTX webpage on http://raspberry_pi_ip:8081

![sdrberry](https://github.com/paulh002/sdrberry/blob/0354b16771afa0fb7ccf55b9a41a7d6fa6422540/images/sdrweb.png)
