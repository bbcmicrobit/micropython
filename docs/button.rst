Buttons
*******

.. py::module:: microbit

There are two buttons on the board, called ``button_a`` and ``button_b``.


Classes
=======

.. py:class:: Button()

    Represents a button.

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
