MicroPython component for the micro:bit
=======================================

MicroPython for the micro:bit.  This currently acts as an application, but
should in the future be a component/library.

Use yotta to build.

Notes to get working:

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
    >>> microbit.display.scroll_string('hello!')
    >>> microbit.random(100)

Tab completion works and is very useful!
