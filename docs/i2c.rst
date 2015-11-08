I²C
***

.. py:module:: microbit.i2c

The ``i2c`` module lets you communicate with devices connected to your board
using the I²C bus protocol.There can be multiple devices connected at the same
time, and each one has its own unique address, that is either fixed for the
device or configured on it. How exactly you should communicate with those
devices, that is, what bytes to send and how to interpret the responses,
depends on the device in question and should be described separately in that
device's documentation.



Functions
=========


.. py:function:: read(addr, n, repeat=False)

    Read ``n`` bytes from the device with address ``addr``. If ``repeat`` is
    ``True``, no stop bit will be sent.


.. py:function:: write(addr, buf, repeat=False)

    Write bytes from ``buf`` to the device with address ``addr``. If ``repeat``
    is ``True``, no stop bit will be sent.
