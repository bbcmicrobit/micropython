MicroPython component for the micro:bit
=======================================

MicroPython for the micro:bit.  This currently acts as an application, but
should in the future be a component/library.

Use yotta to build.

Use target bbc-microbit-classic-gcc-nosd:

```
yt target bbc-microbit-classic-gcc-nosd
```

Run yotta update to fetch remote assets:

```
yt up
```

Start the build:

```
yt build
```

The resulting microbit-micropython.hex file to flash onto the device can be
found in the build/bbc-microbit-classic-gcc-nosd/source from the root of the
repository.

There is a Makefile provided that does some extra preprocessing of the source,
which is needed only if you add new interned strings to qstrdefsport.h.  The
Makefile also includes some convenience targets.

How to use
==========

Upon reset the display should briefly flash a "P" to indicate Python mode.
Then you will have a REPL on the USB CDC serial port, with baudrate 115200.

Then try:

    >>> import microbit
    >>> microbit.display.scroll('hello!')
    >>> microbit.random(100)

Tab completion works and is very useful!
