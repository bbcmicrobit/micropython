I²C
***

.. py:module:: microbit.i2c

The ``i2c`` module lets you communicate with devices connected to your board
using the I²C bus protocol. There can be multiple slave devices connected at
the same time, and each one has its own unique address, that is either fixed
for the device or configured on it. Your board acts as the I²C master.

How exactly you should communicate with the devices, that is, what bytes to
send and how to interpret the responses, depends on the device in question and
should be described separately in that device's documentation.


Functions
=========


.. py:function:: read(addr, n, repeat=False)

    Read ``n`` bytes from the device with address ``addr``. If ``repeat`` is
    ``True``, no stop bit will be sent.


.. py:function:: write(addr, buf, repeat=False)

    Write bytes from ``buf`` to the device with address ``addr``. If ``repeat``
    is ``True``, no stop bit will be sent.


Connecting
----------

Connecting an I²C device to the board is simple -- you have to connect the pin
marked as SCL on the device to pin 19 of the board, and the one marked as SDA
to pin 20. You also need to connect the grounds, and possibly provide power to
the device. You can connect more than one device at once, as long as they have
different addresses.

There are internal pull-up resistors on the I²C lines of the board, but with
particularly long wires or large number of devices you may need to add
additional pull-up resistors, to ensure noise-free communication.
