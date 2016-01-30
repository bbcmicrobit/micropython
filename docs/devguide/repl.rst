.. _dev-repl:

==================
Accessing the REPL
==================

Accessing the REPL on the micro:bit requires:

    * Using a serial communication program
    * Determining the communication port identifier for the micro:bit
    * Establishing communication with the correct settings for your computer

If you are a Windows user you'll need to install the correct drivers. The
instructions for which are found here:

https://developer.mbed.org/handbook/Windows-serial-configuration

Serial communication
--------------------

To access the REPL, you need to select a program to use for serial communication.
Some common options are `picocom` and `screen`. You will need to install
program and understand the basics of connecting to a device.

Determining port
----------------

The micro:bit will have a port identifier (tty, usb) that can be used by the computer for
communicating. Before connecting to the micro:bit, we must determine the port identifier.

Establishing communication with the micro:bit
---------------------------------------------

Depending on your operating system, environment, and serial communication program,
the settings and commands will vary a bit. Here are some common settings for different
systems (please suggest additions that might help others)

**Settings**

* :ref:`Windows <microbit-windows>`
* :ref:`OS X <microbit-osx>`
* :ref:`Linux <microbit-linux>`
* :ref:`Debian and Ubuntu <microbit-debian-ubuntu>`
* :ref:`Red Hat Fedora/CentOS <microbit-redhat>`
* :ref:`Raspberry Pi <microbit-rpi>`
