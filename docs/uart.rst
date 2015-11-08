UART
****

.. py:module:: microbit.uart

The ``uart`` module lets you talk to a device connected to your board using
a serial interface.


Functions
=========

.. method:: init(baudrate=9600, bits=8, parity=None, stop=1, pins=None)

    Initialize serial communication with the specified parameters on the
    specified ``pins``. Note that for correct communication, the parameters
    have to be the same on both communicating devices.

    .. warning::

        Initializing the UART will cause the Python console on USB to become
        unaccessible, as it uses the same hardware. There is currently no way
        to bring the console back, without restarting the module.

    The ``baudrate`` defines the speed of communication. Common baud
    rates include:

        * 9600
        * 14400
        * 19200
        * 28800
        * 38400
        * 57600
        * 115200

    The ``bits`` defines the size of bytes being transmitted, and the board
    only supports 8. The ``parity`` parameter defines how partity is checked,
    and it can be ``None``, ``microbit.uart.ODD`` or ``microbit.uart.EVEN``.
    The ``stop`` parameter tells the number of stop bits, and has to be 1 for
    this board.

    If no ``pins`` are specified, ``microbit.pin0`` is used as the TX pin, and
    ``microbit.pin1`` as the RX pin. You can also specify which pins you want
    by passing a tuple of two pins as ``pins``, the first one being TX, and the
    second one, RX.

    .. note::

        When connecting the device, make sure you "cross" the wires -- the TX
        pin on your board needs to be connected with the RX pin on the device,
        and the RX pin -- with the TX pin on the device. Also make sure the
        ground pins of both devices are connected.


.. method:: uart.any()

   Return ``True`` if any characters waiting, else ``False``.

.. method:: uart.read([nbytes])

   Read characters.  If ``nbytes`` is specified then read at most that many
   bytes.

.. method:: uart.readall()

   Read as much data as possible.

   Return value: a bytes object or ``None`` on timeout.

.. method:: uart.readinto(buf[, nbytes])

   Read bytes into the ``buf``.  If ``nbytes`` is specified then read at most
   that many bytes.  Otherwise, read at most ``len(buf)`` bytes.

   Return value: number of bytes read and stored into ``buf`` or ``None`` on
   timeout.

.. method:: uart.readline()

   Read a line, ending in a newline character.

   Return value: the line read or ``None`` on timeout. The newline character is
   included in the returned bytes.

.. method:: uart.write(buf)

   Write the buffer of bytes to the bus.

   Return value: number of bytes written or ``None`` on timeout.
