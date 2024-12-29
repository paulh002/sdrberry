#!/bin/bash
rm -r sdrweb
rsync -az pi@192.168.88.62:/home/pi/testweb/dist/ sdrweb

