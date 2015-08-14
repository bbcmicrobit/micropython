MicroPython component for the micro:bit
=======================================

MicroPython for the micro:bit.  This currently acts as an application, but
should in the future be a component/library.

Use yotta to build.

Use target bbc-microbit-classic-gcc-nosd:

```
yt target bbc-microbit-classic-gcc-nosd
```

Notes to get working:

- If you're using gcc to compile then you need to use the gcc version of
  CortexContextSwitch.s:

```
$ mv yotta_modules/microbit-dal/source/CortexContextSwitch.s yotta_modules/microbit-dal/source/CortexContextSwitch.s.armcc
$ cp yotta_modules/microbit-dal/source/CortexContextSwitch.s.gcc yotta_modules/microbit-dal/source/CortexContextSwitch.s
```

- You need to manually adjust the allocation policy of microbit-dal's
  MicroBitFiber scheduler:

```
--- old/MicroBitFiber.cpp
+++ new/MicroBitFiber.cpp
@@ -445,11 +445,10 @@
         stackDepth = CORTEX_M0_STACK_BASE - ((uint32_t) __get_MSP());
         bufferSize = oldFiber->stack_top - oldFiber->stack_bottom;
         
-        // If we're too small, increase our buffer exponentially.
+        // If we're too small, increase our buffer conservatively.
         if (bufferSize < stackDepth)
         {
-            while (bufferSize < stackDepth)
-                bufferSize = bufferSize << 1;
+            bufferSize = stackDepth + 8;
                     
             free((void *)oldFiber->stack_bottom);
             oldFiber->stack_bottom = (uint32_t) malloc(bufferSize);
```

How to use
==========

Upon reset the display should briefly flash a "P" to indicate Python mode.
Then you will have a REPL on the USB CDC serial port, with baudrate 115200.

Then try:

    >>> import microbit
    >>> microbit.display.scroll('hello!')
    >>> microbit.random(100)

Tab completion works and is very useful!
