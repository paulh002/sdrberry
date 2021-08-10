# sdrberry
Raspberry pi SDR receiver/ fronted using lvgl gui
!!! This is an ongoing project code only for information purpose only. 
The main goal for this project is to learn about c++ programming, liquid dsp, and for most the use of the GUI toolkit LVGL v8
Functionalit is in beta currently

This source code is still in development.  
Goal is to support Adalm pluto SDR and Radioberry SDR but it should also support other SDR receivers based on SoapySDR.
Also the goal is to support optical encoder and support I2C / serial interface for bandpass filtering
Currently it also supports basic BLE as controler input. I use a ESP32 as BLE server with a service for an optical encoder.

To install you need to configure rasberry pi OS in text only mode, (switch of the GUI)
Compiled using VisualGDB
copy the cfg file (in install directory) to the home dir of the application

ToDo:
- Transmitter part
- Support for Midi contoler
- Setup screen for Network and Wifi
- waterfall display

Done:
- Adalm Pluto
- Radioberry support
- FM broadband
- SSB support
- Basic BLE support


Installation of libraries is necessary:
- Liquid DSP
- Alsa audio
- SoapySDR
- SoapyPlutoSDR (support for Adalm Pluto SDR)
- SoapyRadioBerry (support radioberry)
- libiio
- libad9361-iio
- bluez-5.60
- tinyb
- libpthread.so.0

![sdrberry](https://github.com/paulh002/sdrberry/blob/sdrberry/IMG_20210731_015334.jpgg)

https://www.youtube.com/watch?v=BMJiv3YGv-k
