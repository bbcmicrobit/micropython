MicroPython component for the micro:bit
=======================================

MicroPython for the micro:bit.  This currently acts as an application, but
should in the future be a component/library.

Use yotta to build.

Notes to get working:

- You need to disable the BLE:

```
--- old/MicroBit.cpp
+++ new/MicroBit.cpp
@@ -88,6 +88,12 @@
     // Seed our random number generator
     seedRandom();
 
+    #if 1
+    ble = NULL;
+    ble_firmware_update_service = NULL;
+    ble_device_information_service = NULL;
+    ble_event_service = NULL;
+    #else
     // Start the BLE stack.        
     ble = new BLEDevice();
     
@@ -108,6 +114,7 @@
     ble->setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
     ble->setAdvertisingInterval(Gap::MSEC_TO_ADVERTISEMENT_DURATION_UNITS(1000));
     ble->startAdvertising();  
+    #endif
 
     // Start refreshing the Matrix Display
     systemTicker.attach(this, &MicroBit::systemTick, MICROBIT_DISPLAY_REFRESH_PERIOD);     
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
    >>> microbit.display.scroll_string('hello!')
    >>> microbit.random(100)

Tab completion works and is very useful!
