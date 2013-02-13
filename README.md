# libnxtusb
C library for interfacing with [LEGO NXT](http://mindstorms.lego.com) brick

## Features 
Supports full set of direct commands through usb.

## License
WTFPL. Just do what the fuck you want.

## Installation
You'll need libusb-1.0

    mkdir build
    cd build
    cmake ..
    make

## Documentation
You'll need doxygen for this.
Simply run

    doxygen doxygen.cfg

Resulting documentation will be in html folder.

## Notes
* This library is by no means of production quality.
* Author are not in any way affilated with Lego.
* It's only been tested on Linux (x86-64 and ARM).
* It works for me.
* You can do with this code whatever you want, even print it on toilet paper.

## Why
Because libnxtpp is shit, dependent on old libusb

Same goes for perl LEGO::NXT and Device::USB, duh.

## TODO
* Write C++ OO-wrapper
* Write Perl module
* Grab a glass of single-malt whiskey
