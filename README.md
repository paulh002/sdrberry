# sdrberry
Raspberry pi SDR receiver using lvgl gui
!!! This is an ongoing project code only for information purpose only. 
The main goal for this project is to learn about c++ programming, liquid dsp, and for most the use of the GUI toolkit LVGL v8
Functionalit is in beta currently

This source code is still in development.  
Also it should support other SDR receivers like RadioBerry / Hermes light2
Also the goal is to support optical encoder and support I2C / serial interface for bandpass filtering
Currently it also supports basic BLE as controler input. I use a ESP32 as BLE server suppling optical encoder support

To install you need to configure rasberry pi OS in text only mode, (switch of the GUI)
Compiled using VisualGDB
copy the cfg file (in install directory) to the home dir of the application

ToDo:
- Transmitter part
- Support for Midi contoler
- Setup screen for Network and Wifi
- waterfall display

Done:
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




https://www.youtube.com/watch?v=ds-n-7sHXl8&t=2s
