.. BBC Microbit Micropython documentation master file, created by
   sphinx-quickstart on Tue Oct 20 10:41:30 2015.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

BBC micro:bit MicroPython documentation
=======================================

Welcome!

The `BBC micro:bit <https://microbit.org>`_ is a small computing device for
children. One of the languages it understands is the popular Python programming
language. The version of Python that runs on the BBC micro:bit is called
MicroPython.

This documentation includes lessons for teachers
and API documentation for developers (check out the index on the left). We hope
you enjoy developing for the BBC micro:bit using MicroPython.

If you're a new programmer, teacher or unsure where to start, begin with the
:ref:`Tutorials <Tutorials>` and use the `micro:bit Python Editor <https://python.microbit.org>`_
to program the micro:bit. 

.. note::

   The MicroPython API will not work in the MakeCode editor, as this
   uses a `different version of Python. <https://support.microbit.org/support/solutions/articles/19000111744-makecode-python-and-micropython>`_

.. image:: comic.png

Projects related to MicroPython on the BBC micro:bit include:

* `micro:bit Python Editor <https://python.microbit.org>`_ - A simple
  browser-based code editor, developed by the Micro:bit Educational
  Foundation  and designed to help teachers and learners get the
  most out of text-based programming on the micro:bit.

* `Mu <https://codewith.mu>`_ - A simple offline code editor for kids, teachers
  and beginner programmers.

* `uFlash <https://uflash.readthedocs.io/en/latest/>`_ - A command line tool
  for flashing raw Python scripts onto a BBC micro:bit.

.. toctree::
    :maxdepth: 2
    :caption: Tutorials

    tutorials/introduction
    tutorials/hello
    tutorials/images
    tutorials/buttons
    tutorials/io
    tutorials/music
    tutorials/random
    tutorials/movement
    tutorials/gestures
    tutorials/direction
    tutorials/storage
    tutorials/speech
    tutorials/network
    tutorials/radio
    tutorials/next

.. toctree::
   :maxdepth: 2
   :caption: API Reference

   microbit_micropython_api.rst
   microbit.rst
   accelerometer.rst
   audio.rst
   ble.rst
   button.rst
   compass.rst
   display.rst
   filesystem.rst
   i2c.rst
   image.rst
   machine.rst
   micropython.rst
   music.rst
   neopixel.rst
   os.rst
   pin.rst
   radio.rst
   random.rst
   speech.rst
   spi.rst
   uart.rst
   utime.rst

.. toctree::
   :maxdepth: 2
   :caption: Developer Guide

   devguide/flashfirmware
   devguide/repl
   devguide/hexformat
   devguide/contributing

.. toctree::
   :maxdepth: 2
   :caption: Indices and tables

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`
