Buttons
*******

.. py::module:: microbit

There are two buttons on the board, called ``button_a`` and ``button_b``.

Attributes
==========


.. py:attribute:: button_a

    A ``Button`` instance (see below) representing the left button.


.. py:attribute:: button_b

    Represents the right button.


Classes
=======

.. py:class:: Button()

    Represents a button.

    .. note::
        This class is not actually available to the user, it is only used by
        the two button instances, which are provided already initialized.


    .. py:method:: is_pressed()

        Returns ``True`` if the specified button ``button`` is pressed, and
        ``False`` otherwise.

Example
=======

.. code::

    import microbit

    while True:
        if microbit.button_a.is_pressed() and microbit.button_b.is_pressed():
            microbit.display.scroll("AB")
            break
        elif microbit.button_a.is_pressed():
            microbit.display.scroll("A")
        elif microbit.button_b.is_pressed():
            microbit.display.scroll("B")
        microbit.sleep(100)
