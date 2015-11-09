Image
*****

.. py:module:: microbit

Classes
=======

.. py:class::
    Image(string)
    Image(width=None, height=None, buffer=None)

    If ``string`` is used, it has to consist of digits 0-9 arranged into
    lines, describing the image, for example::

        image = Image("90009\n"
                      "09090\n"
                      "00900\n"
                      "09090\n"
                      "90009")

    will create a 5×5 image of an X.

    With the other form, it creates an empty image with ``width`` columns and
    ``height`` rows. Optionally ``buffer`` can be an array of
    ``width``×``height`` integers in range 0-9 to initialize the image.


    .. py:method:: width()

        Return the number of columns in the image.


    .. py:method:: height()

        Return the numbers of rows in the image.


    .. py:method:: set_pixel(x, y, value)

        Set the brightness of the pixel at column ``x`` and row ``y`` to the
        ``value``, which has to be between 0 (dark) and 9 (bright).

        This method will raise an exception when called on any of the build-in
        read-only images, like ``Image.HEART``.


    .. py:method:: get_pixel(x, y)

        Return the brightness of pixel at column ``x`` and row ``y`` as an
        integer between 0 and 9.


    .. py:method:: shift_left(n)

        Return a new image created by shifting the picture left by ``n``
        columns.


    .. py:method:: shift_right(n)

        Same as ``image.shift_left(-n)``.
g

    .. py:method:: shift_up(n)

        Return a new image created by shifting the picture up by ``n`` rows.


    .. py:method:: shift_down(n)

        Same as ``image.shift_up(-n)``.


Attributes
==========

The ``Image`` class also has the following instances of itself included as its
attributes:

    * ``Image.HEART``
    * ``Image.HEART_SMALL``
    * ``Image.HAPPY``
    * ``Image.SAD``
    * ``Image.CONFUSED``
    * ``Image.ANGRY``
    * ``Image.ASLEEP``
    * ``Image.SURPRISED``
    * ``Image.YES``
    * ``Image.NO``
    * ``Image.CLOCK12``, ``Image.CLOCK11``, ``Image.CLOCK10``, ``Image.CLOCK9``,
      ``Image.CLOCK8``, ``Image.CLOCK7``, ``Image.CLOCK6``, ``Image.CLOCK5``,
      ``Image.CLOCK4``, ``Image.CLOCK3``, ``Image.CLOCK2``, ``Image.CLOCK1``
    * ``Image.ARROW_N``, ``Image.ARROW_NE``, ``Image.ARROW_E``,
      ``Image.ARROW_SE``, ``Image.ARROW_S``, ``Image.ARROW_SW``,
      ``Image.ARROW_W``, ``Image.ARROW_NW``


Operations
==========

.. code::

    repr(image)

Get a compact string representation of the image.

.. code::

    str(image)

Get a readable string representation of the image.

.. code::

    image1 + image2

Create a new image by adding the brightness values from the two images for
each pixel.

.. code::

    image + n

Create a new image by multiplying the brightness of each pixel by ``n``.
