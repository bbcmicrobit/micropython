Bluetooth
*********

micro:bit V1
============

While the BBC micro:bit has hardware capable of allowing the device to work as
a Bluetooth Low Energy (BLE) device, it only has 16k of RAM. The BLE stack
alone takes up 12k RAM which means there's not enough memory for MicroPython
to support Bluetooth on a micro:bit V1.

.. note::
    MicroPython uses the radio hardware with the :mod:`radio` module. This
    allows users to create simple yet effective wireless networks of micro:bit
    devices.

    Furthermore, the protocol used in the :mod:`radio` module is a lot simpler
    than BLE, making it far easier to use in an educational context.

micro:bit V2
============

The nRF52833 used by the micro:bit V2 has 128k of RAM, allowing Micropython to make
use of the BLE stack. Currently the only implemented feature is BLE flashing, allowing 
a user to update the firmware on the micro:bit over Bluetooth.

At the time that this was written the `Nordic DFU service <https://infocenter.nordicsemi.com/topic/sdk_nrf5_v16.0.0/lib_bootloader_dfu_process.html>`_ is implemented, and partial flashing is currently working but in
beta. The Nordic DFU service updates everything in flash and will take a (relatively) long
time to complete, whereas the partial flashing service only updates the filesystem containing
the user scripts.
