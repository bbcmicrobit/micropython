Neopixel
****

.. py:module:: neopixel

The ``neopixel`` module lets you use Neopixel (WS2812) individually addressable RGB LED strips with the Microbit.    
Note to use the ``neopixel`` module, you need to import it separately with::

    import neopixel

.. warning::
    
    Do not use the 3v connector on the Microbit to power any more than 8 Neopixels at a time.
    If you wish to use more than 8 Neopixels, you must use a separate 3v-5v power supply for the Neopixel power pin.
    
.. note::
    
    From our tests, the Microbit Neopixel module can drive up to around 256 Neopixels. 
    Anything above that and you may experience weird bugs and issues.   

Functions
=========

.. method:: neopixel.Neopixel(pin, length)

    Initialize Neopixel strip on the specified pin and with the specified length.

.. method:: Neopixel.clear()

   Clears the entire Neopixel stip by setting each pixel to (0, 0, 0).

.. method:: Neopixel.show()

   Pushes the colour data to the strip. Must be done to display any changes.
.. note::
    If you aren't seeing anything display on your Neopixel stip, make sure you are
    calling .show() at least somewhere or else your updates will not be shown.

Using the Neopixel module
=========================

You can interact with Neopixels similar to a list of tuples. After importing the module and
setting up your strip::

    from microbit import *
    import neopixel
    np = neopixel.NeoPixel(pin0, 8)
    
You can set your pixels just like a list. To set the first pixel to full brightness red, you would use::

    np[0] = (255, 0, 0)

Or the final pixel to purple::
    
    np[-1] = (255, 0, 255)

You can then for example, get the current colour value of the first pixel with::
    
    print(np[0])

Finally, to push the new colour data to your Neopixel strip, use the .show() function::
    
    np.show()

Example
=======

.. include:: ../examples/neopixel_random.py
    :code: python
