# sdrberry
Raspberry pi SDR receiver using lvgl gui
!!! This is an ongoing project code only for information purpose only. 
Functionalit is in beta currently

This source code is still in development. Currently only broadband FM is supported.
Next step is to add the other modes like SSB 
Also it should support other SDR receivers like RadioBerry / Hermes light2
Also the goal is to support optical encoder and support I2C / serial interface for bandpass filtering

To install you need to configure rasberry pi OS in text only mode, (switch of the GUI)
Compiled using VisualGDB

Installation of libraries is necessary:
- Liquid DSP
- Alsa audio
- SoapySDR
- SoapyPlutoSDR
- SoapyRadioBerry
- libiio
- libad9361-iio
- bluez-5.60
- tinyb
- libpthread.so.0

https://www.youtube.com/watch?v=ds-n-7sHXl8&t=2s
