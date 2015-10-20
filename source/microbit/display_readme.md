# Notes on the Display

Rendering of images to the display is now done entirely within MicroPython
without using the DAL's rendering logic.

This achieves the following:

1. It gives more obviously distinct brightness levels on the scale of 1 to 9
2. Most obviously, the dimmest level is dimmer. Level 1 is clearly dimmer than level 2
3. It is possible to support sophisticated animations asynchronously.

## How rendering works

Rendering on the microbit display works by using pulse width modulation implemented
in software. A render cycle consists of:

* Render each display row (which does not correspond to the image row)
    * Turn off all LEDs in the previous row.
    * Turn on all LEDs that are maximum brightness
    * Do any computation required to update the image
    * Turn on all LEDs with non-zero brightness
    * In exponentially increasing time steps:
        * Turn off LEDs in increasing order of brightness.
    
This means that each LEDs is turned on for a period of time approximately proportional 
to 2**brightness.
By turning on maximum brightness LEDs before updating the image, and performing the 
increasing time steps after the update, image is rendering is smooth even with complex 
image iterators.

Provided that the display update step takes no more that about 2.2ms then
there will no effect on the rendering of the image.
Even if it takes up to 4ms (which a lot of computation to yield just a single image) 
then only effect is that level 8 brightness will be dimmed toward the level 7 brightness.

## How this differs from the DAL.
The DAL updates the image before turning on any pixels. 
DAL rendering timings assume that the full 6ms cycle duration can be divided
up evenly. This does not reflect the underlying clock speed and may be the cause 
of the unevenness in brightness levels.
The DAL supports 255 brightness levels as well as rotation in the rendering ticker function,
which adds quite a lot of overhead to a function that is called more than 1000 times a second.