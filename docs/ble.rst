Bluetooth
*********

While the BBC micro:bit has hardware capable of allowing the device to work as
a Bluetooth Low Energy (BLE) device, it only has 16k of RAM. The BLE stack
alone takes up 12k RAM which means there's not enough room to run MicroPython.

Future versions of the device may come with 32k RAM which would be sufficient.
However, until such time it's highly unlikely MicroPython will support BLE.

.. note::
    MicroPython uses the radio hardware with the ``radio`` module. This allows
    users to create simple yet effective wireless networks of micro:bit
    devices.

    Furthermore, the protocol used in the ``radio`` module is a lot simpler
    than BLE, making it far easier to use in an educational context.
