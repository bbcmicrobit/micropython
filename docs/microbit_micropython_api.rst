micro:bit Micropython API
*************************

.. warning::
    This API is subject to frequent changes. This page reflects the current micro:bit API in a developer-friendly (but not necessarily kid-friendly) way.

The microbit module
===================

Everything lives in the `microbit` module.  For ease of use it's recommended to do `from microbit import *`.  The below documentation assumes you have done this.

There are a few functions available directly::

    random(n) # returns a pseudo-random number between 0 and n-1
    sleep(ms) # sleep for the given number of milliseconds
    running_time() # returns the number of milliseconds since the micro:bit was last switched on.
    panic([error_code]) # makes the micro:bit enter panic mode (this usually happens when the DAL runs out of memory, and causes a sad face to be drawn on the display)
    reset() # resets the micro:bit

The rest of the functionality is provided by objects and classes in the microbit module, as described below.

Note that the API exposes integers only (ie no floats are needed, but they may be accepted).  We thus use milliseconds for the standard time unit.

Buttons
-------

There are 2 buttons::

    button_a
    button_b

These are both objects and have the following methods::

    button.is_pressed() # returns True or False

The LED display
---------------

The LED display is exposed via the `display` object::

    display.get_pixel(x, y) # gets the brightness of the pixel (x,y)
    display.set_pixel(x, y, val) # sets the brightness of the pixel (x,y) to val (between 0 and 9, inclusive)
    display.clear() # clears the display
    display.show(image, delay=0, wait=True, loop=False, clear=False) # shows the image
    display.show(iterable, delay=400, wait=True, loop=False, clear=False) # shows each image or letter in the iterable, with delay ms. in between each.
    display.scroll(string, delay=400) # scrolls a string across the display (more exciting than display.show)

Pins
----

Provide digital and analog input and output functionality, for the pins in the connector. Some pins are connected internally to the I/O that drives the LED matrix and the buttons.

Each pin is provided as an object directly in the ``microbit`` module.  This keeps the API relatively flat, making it very easy to use:

    * pin0
    * pin1
    * ...
    * pin15
    * pin16
    * *Warning: P17-P18 (inclusive) are unavailable.*
    * pin19
    * pin20

Each of these pins are instances of the ``MicroBitPin`` class, which offers the following API::

    pin.write_digital(value) # value can be 0, 1, False, True
    pin.read_digital() # returns either 1 or 0
    pin.write_analog(value) # value is between 0 and 1023
    pin.read_analog() # returns an integer between 0 and 1023
    pin.set_analog_period_microseconds(int) # sets the period of the PWM output of the pin in microseconds (see https://en.wikipedia.org/wiki/Pulse-width_modulation)
    pin.is_touched() # returns boolean

Images
------

.. note::

    You don't always need to create one of these yourself - you can access the
    image shown on the display directly with `display.image`. `display.image`
    is just an instance of `Image`, so you can use all of the same methods.

Images API::

    # constructor overloads:
    image = Image() # creates an empty 5x5 image
    image = Image('90009\n09090\n00900\n09090\n90009') # create an image from a string - each character in the string represents an LED - 0 (or space) is off and 9 is maximum brightness.
    image = Image(width, height) # create an empty image of given size
    image = Image(width, height, buffer) # initialises an Image with the specified width and height. The buffer should be an array of length width * height

    # methods
    image.width() # returns the image's width (most often 5)
    image.height() # returns the image's height (most often 5)
    image.set_pixel(x, y, value) # sets the pixel at the specified position (between 0 and 9). May fail for constant images.
    image.get_pixel(x, y) # gets the pixel at the specified position (between 0 and 9)
    image.shift_left(n) # returns a new image created by shifting the picture left 'n' times.
    image.shift_right(n) # returns a new image created by shifting the picture right 'n' times.
    image.shift_up(n) # returns a new image created by shifting the picture up 'n' times.
    image.shift_down(n) # returns a new image created by shifting the picture down 'n' times.
    repr(image) # get a compact string representation of the image
    str(image) # get a more readable string representation of the image

    #operators
    image + image # returns a new image created by superimposing the two images
    image * n # returns a new image created by multiplying the brightness of each pixel by n

    # constants (currently, just lots of images)
    Image.HEART
    Image.HEART_SMALL
    Image.HAPPY
    Image.SAD
    Image.CONFUSED
    Image.ANGRY
    Image.ASLEEP
    Image.SURPRISED
    Image.YES
    Image.NO
    Image.CLOCK12 # clock at 12 o' clock
    Image.CLOCK11
    ... # many clocks (Image.CLOCKn)
    Image.CLOCK1 # clock at 1 o'clock
    Image.ARROW_N
    ... # arrows pointing N, NE, E, SE, S, SW, W, NW (microbit.Image.ARROW_direction)
    Image.ARROW_NW

The accelerometer
-----------------

The accelerometer is accessed via the ``accelerometer`` object::

    accelerometer.get_x() # [WILL CHANGE]
    accelerometer.get_y() # [WILL CHANGE]
    accelerometer.get_z() # [WILL CHANGE]

The compass
-----------

The compass is access via the `compass` object::

    compass.heading()
    compass.calibrate()
    compass.is_calibrating()
    compass.is_calibrated()

I2C bus
-------

There is an I2C bus on the micro:bit that is exposed via the `i2c` object.  It has the following methods::

    i2c.read(addr, n, repeat=False) # read n bytes from device with addr; repeat=True means a stop bit won't be sent
    i2c.write(addr, buf, repeat=False) # write buf to device with addr; repeat=True means a stop bit won't be sent
