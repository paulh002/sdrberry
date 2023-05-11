# sdrberry
Raspberry pi SDR tranceiver / fronted using lvgl gui
!!! This is an ongoing project code only for information purpose only. 
The main goal for this project is to learn about c++ programming, liquid dsp, and for most the use of the GUI toolkit LVGL v8
Functionalit is in beta currently

This source code is still in development.  
Goal is to support Adalm pluto SDR, Radioberry, RTL-SDR, SDRPlay etc but it should also support other SDR receivers based on SoapySDR.
Also the goal is to support optical encoder and support I2C / serial interface for bandpass filtering
I use a ESP32 as CAT controler with a service for an optical encoder.
Alternative a Contour Shuttle express can be used as rotary controler. 

To install you need to configure rasberry pi OS in text only mode, (switch off the GUI)
Compiled using VisualGDB or cmake, gcc and gfortran (for wsjtx_lib which includes the wsjtx fortran code of Joe Taylor)
copy the cfg file (in install directory) to the home dir

installation instruction in install_guide.txt
install script install.sh installs all components based on a fresh raspberry pi SD card (OS should be updated and in CLI mode)

This software makes use of opensource libraries like:
- Noise reduction code is an port from DD4WH https://github.com/df8oe/UHSDR/wiki/Noise-reduction  adapted for raspberry pi
- WSJTX fortran library of Joe Taylor K1JT
- Liquid-dsp of Joseph D. Gaeddert
- Lvgl of Gabor Kiss-Vamosi
- and others

ToDo:
- Support for Midi contoler
- Setup screen for Network and Wifi
- waterfall display
- Different noise reduction schemas like lms etc.

Done:
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

Installation of libraries is necessary:
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

## DSI display 7inch  
In the directory "install/bullseye-7inch rpi" are example config.txt and cmdline.txt files for rotating the screen
5 inch DSI screens from waveshare do not need any adjustment

![sdrberry](https://github.com/paulh002/sdrberry/blob/master/rb_tranceiver.jpg)

![sdrberry](https://github.com/paulh002/sdrberry/blob/master/IMG_20220203_ft8.jpg)

![sdrberry](https://github.com/paulh002/sdrberry/blob/master/IMG_20211215_200645.jpg)

![sdrberry](https://github.com/paulh002/sdrberry/blob/master/IMG_20210909_183113.jpg)

[![Radioberry demo](https://img.youtube.com/vi/BMJiv3YGv-k/0.jpg)](https://youtu.be/PQ_Np5SfcxA)

# ESP32 Remote control for raspberry pi
![sdrberry](https://github.com/paulh002/sdrberry/blob/master/IMG_20210903_133827.jpg)