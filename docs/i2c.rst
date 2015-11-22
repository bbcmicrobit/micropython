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

You should connect the device's ``SCL`` pin to micro:bit pin 19, and the
device's ``SDA`` pin to micro:bit pin 20. You also must connect the device's
ground to the micro:bit ground (pin ``GND``). You may need to power the device
using an external power supply or the micro:bit.

There are internal pull-up resistors on the I²C lines of the board, but with
particularly long wires or large number of devices you may need to add
additional pull-up resistors, to ensure noise-free communication.