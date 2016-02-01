SPI
***

.. py:module:: microbit.spi

The ``spi`` module lets you talk to a device connected to your board using
a serial peripheral interface (SPI) bus. SPI uses a so-called master-slave
architecture with a single master. You will need to specify the connections
for three signals:

* SCLK : Serial Clock (output from master).
* MOSI : Master Output, Slave Input (output from master).
* MISO : Master Input, Slave Output (output from slave).


Functions
=========

.. method:: init(baudrate=1000000, bits=8, mode=0, sclk=pin13, mosi=pin15, miso=pin14)

    Initialize SPI communication with the specified parameters on the
    specified ``pins``. Note that for correct communication, the parameters
    have to be the same on both communicating devices.

    The ``baudrate`` defines the speed of communication.

    The ``bits`` defines the size of bytes being transmitted. Currently only
    ``bits=8`` is supported. However, this may change in the future.

    The ``mode`` determines the combination of clock polarity and phase
    according to the following convention, with polarity as the high order bit
    and phase as the low order bit:

    +----------+----------+-------+
    | SPI Mode | Polarity | Phase |
    +==========+==========+=======+
    | 0        | 0        | 1     |
    +----------+----------+-------+
    | 1        | 0        | 0     |
    +----------+----------+-------+
    | 2        | 1        | 1     |
    +----------+----------+-------+
    | 3        | 1        | 0     |
    +----------+----------+-------+

    The ``sclk``, ``mosi`` and ``miso`` arguments specify the pins to use for
    each type of signal.

.. method:: spi.read(nbytes)

   Read at most ``nbytes``. Returns what was read.

.. method:: spi.write(buffer)

   Write the ``buffer`` of bytes to the bus.

.. method:: spi.write_readinto(out, in)

   Write the ``out`` buffer to the bus and read any response into the ``in``
   buffer. The length of the buffers should be the same. The buggers can be
   the same object.
