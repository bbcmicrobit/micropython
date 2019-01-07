.. _install-dev:

============
Installation
============

This section will help you set up the tools and programs needed for
developing programs and firmware to flash to the BBC micro:bit using MicroPython.

Dependencies
------------


Development Environment
-----------------------

You will need:

* git
* yotta

Depending on your operating system, the installation instructions vary. Use
the installation scenario that best suits your system.

Yotta will require an ARM mbed account.  It will walk you through signing up
if you are not registered.

Installation Scenarios
----------------------

* :ref:`Windows <microbit-windows>`
* :ref:`OS X <microbit-osx>`
* :ref:`Linux <microbit-linux>`
* :ref:`Debian and Ubuntu <microbit-debian-ubuntu>`
* :ref:`Red Hat Fedora/CentOS <microbit-redhat>`
* :ref:`Raspberry Pi <microbit-rpi>`


.. _microbit-windows:

Windows
~~~~~~~

When installing `Yotta
<http://yottadocs.mbed.com/>`_, make sure you have these components ticked to install.

- python
- gcc
- cMake
- ninja
- Yotta
- git-scm
- mbed serial driver



.. _microbit-osx:

OS X
~~~~

After cloning the repository, install yotta using the app and launch the app.
You can find the latest release of yotta for OS X at
`<https://github.com/ARMmbed/yotta_osx_installer/releases/>`_ .


.. _microbit-linux:

Linux
~~~~~

These steps will cover the basic flavors of Linux and working with the
micro:bit and MicroPython. See also the specific sections for Raspberry Pi,
Debian/Ubuntu, and Red Hat Fedora/Centos.


.. _microbit-debian-ubuntu:

Debian and Ubuntu
^^^^^^^^^^^^^^^^^

The following commands should install the toolchain on a modern Debian/Ubuntu
variant::

  sudo add-apt-repository -y ppa:team-gcc-arm-embedded
  sudo apt-get update
  sudo apt-get install gcc-arm-embedded
  sudo apt-get install cmake ninja-build srecord libssl-dev
  sudo -H pip3 install yotta

In order to make development life a bit easier, you can (on Ubuntu) add yourself
to the ``dialout`` group, which grants write access to the serial device
``/dev/ttyACM0``::

  sudo usermod -a -G dialout ${USER}

You may need to relogin in order for the changes to groups to take effect.

.. _microbit-redhat:

Red Hat Fedora/CentOS
^^^^^^^^^^^^^^^^^^^^^


.. _microbit-rpi:

Raspberry Pi
^^^^^^^^^^^^



.. _next-steps:

Next steps
----------

Congratulations. You have installed your development environment and are ready to
begin :ref:`flashing firmware <flashfirmware>`  to the micro:bit.
