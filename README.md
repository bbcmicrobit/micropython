MicroPython for the BBC micro:bit
=================================

This is the source code for MicroPython running on the BBC micro:bit!

To get involved with the micro:bit community join the Slack channel by signing up here:
https://tech.microbit.org/get-involved/where-to-find/

Various things are in this repository, including:
- Source code in source/ and inc/ directories.
- Example Python programs in the examples/ directory.
- Tools in the tools/ directory.

The source code is a yotta application and needs yotta to build, along
with an ARM compiler toolchain (eg arm-none-eabi-gcc and friends).

Ubuntu users can install the needed packages using:
```
sudo add-apt-repository -y ppa:team-gcc-arm-embedded
sudo apt-get update
sudo apt-get install gcc-arm-embedded
sudo apt-get install cmake ninja-build srecord libssl-dev
sudo -H pip3 install yotta
```

Once all packages are installed run
`git clone https://github.com/bbcmicrobit/micropython` to get the source code.
The move to the created directory and use yotta and the provided Makefile to build.
You might need need an Arm Mbed account to complete some of the yotta commands,
if so, you could be prompted to create one as a part of the process.

- Use target [bbc-microbit-classic-gcc-nosd](https://github.com/lancaster-university/yotta-target-bbc-microbit-classic-gcc-nosd)
from GitHub (the yotta registry is deprecated as of 2021):

  ```
  yotta target bbc-microbit-classic-gcc-nosd@https://github.com/lancaster-university/yotta-target-bbc-microbit-classic-gcc-nosd
  ```

- Run yotta update to fetch remote assets:

  ```
  yotta up
  ```

- Start the build:

  ```
  make all
  ```

The resulting firmware.hex file to flash onto the device can be
found in the build/ directory from the root of the repository.

The Makefile provided does some extra preprocessing of the source,
adds version information to the UICR region, puts the resulting
firmware at build/firmware.hex, and includes some convenience targets.

How to use
==========

Upon reset you will have a REPL on the USB CDC serial port, with baudrate
115200 (eg picocom /dev/ttyACM0 -b 115200).

Then try:

    >>> import microbit
    >>> microbit.display.scroll('hello!')
    >>> microbit.button_a.is_pressed()
    >>> dir(microbit)

Tab completion works and is very useful!

Read our documentation here:

https://microbit-micropython.readthedocs.io/en/latest/

You can also use the tools/pyboard.py script to run Python scripts directly
from your PC, eg:

    $ ./tools/pyboard.py /dev/ttyACM0 examples/conway.py

Be brave! Break things! Learn and have fun!
