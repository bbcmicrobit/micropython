Bluetooth
*********

While the BBC micro:bit has hardware capable of allowing the device to work as
a Bluetooth Low Energy (BLE) device, v1 only has 16k of RAM (v2: 128k). The BLE stack
alone takes up 12k RAM which means there's not enough memory for MicroPython
to support Bluetooth for v1.

.. note::
    MicroPython uses the radio hardware with the :mod:`radio` module. This
    allows users to create simple yet effective wireless networks of micro:bit
    devices.

    Furthermore, the protocol used in the :mod:`radio` module is a lot simpler
    than BLE, making it far easier to use in an educational context.
