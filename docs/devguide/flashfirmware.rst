.. _flashfirmware:

=================
Flashing Firmware
=================

Building firmware
-----------------
Use yotta to build.

Use target bbc-microbit-classic-gcc-nosd::

  yotta target bbc-microbit-classic-gcc-nosd

Run yotta update to fetch remote assets::

  yotta up

Start the build with either yotta::

  yotta build

...or use the Makefile::

  make all

The result is a microbit-micropython hex file (i.e. ``microbit-micropython.hex``)
found in the build/bbc-microbit-classic-gcc-nosd/source from the root of the
repository.

The Makefile does some extra preprocessing of the source, which is needed only
if you add new interned strings to ``qstrdefsport.h``. The Makefile also puts
the resulting firmware at build/firmware.hex, and includes some convenience
targets.

Preparing firmware and a Python program
---------------------------------------

A script in the following location, called using the following syntax, should
help::

  tools/makecombinedhex.py <firmware.hex> <script.py> [-o <combined.hex>]

The script will output to ``stdout`` if no output option (``-o``) is provided.

Flashing to the micro:bit
-------------------------

The Microbit mounts itself as a mass storage device over USB.  When it detects
that a .hex file has been uploaded to the internal mass storage device, it will
flash itself with the bytecode representation, and start running the program.

This means that uploading a .hex file should result in the Microbit running your
code.



**Installation Scenarios**

* :ref:`Windows <microbit-windows>`
* :ref:`OS X <microbit-osx>`
* :ref:`Linux <microbit-linux>`
* :ref:`Debian and Ubuntu <microbit-debian-ubuntu>`
* :ref:`Red Hat Fedora/CentOS <microbit-redhat>`
* :ref:`Raspberry Pi <microbit-rpi>`
