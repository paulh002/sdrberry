#!/bin/bash
echo "pi ALL=(ALL) NOPASSWD: ALL" >> 010_pi-nopasswd
sudo cp  010_pi-nopasswd /etc/sudoers.d/.
sudo chmod 0440 /etc/sudoers.d/010_pi-nopasswd