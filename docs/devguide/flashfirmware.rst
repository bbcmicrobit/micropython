.. _flashfirmware:

=================
Flashing Firmware
=================

Building firmware
-----------------
Use yotta to build.

Use target bbc-microbit-classic-gcc-nosd::

  yt target bbc-microbit-classic-gcc-nosd

Run yotta update to fetch remote assets::

  yt up

Start the build with either yotta::

  yt build

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

tools/makecombined

hexlify



Flashing to the micro:bit
-------------------------




**Installation Scenarios**

* :ref:`Windows <microbit-windows>`
* :ref:`OS X <microbit-osx>`
* :ref:`Linux <microbit-linux>`
* :ref:`Debian and Ubuntu <microbit-debian-ubuntu>`
* :ref:`Red Hat Fedora/CentOS <microbit-redhat>`
* :ref:`Raspberry Pi <microbit-rpi>`
