Bluetooth
*********

While the BBC micro:bit has Bluetooth Low Energy (BLE) hardware it only has
16k of RAM. The BLE stack alone takes up 12k RAM which means there's not enough
room to (currently) run MicroPython.

Future versions of the device may come with 32k RAM which would be sufficient.
However, until such time it's highly unlikely MicroPython will support BLE.
