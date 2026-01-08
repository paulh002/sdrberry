# sdrberry and radioberry on pi5 
To use radioberry on the pi5 there is a kernel patch neccessay. This patch is supplied as a pull request to raspberry pi kernel repo by Johan PA2GSB.
The files in this directory descibe the steps and files to patch the kernel your self.

kernel-compile.txt descibe all the steps
pio_rp1.h and rp1-pio.c are the changed files
6927.patch.txt contains the changed code as a git patch
radioberry.rbf is the gateware that need this patch.