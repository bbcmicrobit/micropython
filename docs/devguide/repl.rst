.. _dev-repl:

==================
Accessing the REPL
==================

REPL (Read-Evaluate-Print-Loop) allows the micro:bit to read and evaluate code
in real-time as you write it.


Using the micro:bit Python Editor
---------------------------------

The browser-based Python editor has built-in REPL support, that can be accessed
using `WebUSB <https://web.dev/usb/>`_.
You can read more about how WebUSB is used in the editors in this article on
`direct flashing from the browser in the micro:bit apps and editors <https://support.microbit.org/support/solutions/articles/19000084059>`_.

To use WebUSB, you will need a Google Chrome based browser and a micro:bit with
`firmware <https://microbit.org/get-started/user-guide/firmware/>`_ at version
**0249** or above.

To use the REPL:

1. Flash a Python program to the micro:bit, if you have not done so already.
2. Select Open Serial to open the REPL window.
3. Click the blue bar to ``Send CTRL-C for REPL`` or press ``CTRL`` + ``C`` on
   your keyboard to enter the REPL.


Using a serial communication program
------------------------------------

The `Mu Editor <https://codewith.mu/en/tutorials/1.1/repl>`_ has built-in
support for REPL and even includes a real-time data plotter.

`microREPL <https://github.com/ntoll/microrepl>`_ is an easy to use python
serial terminal that detects and connects to the micro:bit automatically.
To exit microREPL press ``CTRL`` + ``]`` on your keyboard.

Some other common options are `picocom` and `screen`. You will need to install
a program and read the appropriate documentation to understand the basics of
connecting to a device.


Determining the port
--------------------

Accessing the REPL on the micro:bit will require you to:

* Determine the communication port identifier for the micro:bit
* Use a program to establish communication with the device

The micro:bit will have a port identifier (tty, usb) that can be used by the
computer for communicating. Before connecting to the micro:bit we must
determine the port identifier.

Windows
~~~~~~~

When you have installed the aforementioned drivers the micro:bit will appear in
device-manager as a COM port.

macOS
~~~~~

Open Terminal and type ``ls /dev/cu.*`` to see a list of connected serial
devices; one of them will look like ``/dev/cu.usbmodem1422`` (the exact number
will depend on your computer).

Linux
~~~~~

In terminal, type ``dmesg | tail`` which will show which ``/dev`` node the
micro:bit was assigned (e.g. ``/dev/ttyUSB0``).


Communicating with the micro:bit
--------------------------------

Once you have found the port identifier you can use a serial terminal program
to communicate with the micro:bit.

Windows
~~~~~~~

You may wish to use Tera Term, PuTTY, or another program.

In Tera Term:
	* Plug in the micro:bit and open Tera Term
	* Select Serial as the port
	* Go to Setup -> Serial port. Ensure the Port is the correct COM port.
	* Choose a baud rate of ``115200``, data 8 bits, parity none, stop 1 bit.

In PuTTY:
	* Plug in the micro:bit and open PuTTY
	* Switch the Connection Type to Serial
	* Ensure the Port is the correct COM port
	* Change the baud rate to ``115200``
	* Select 'Serial' on the menu on the left, then click 'Open'


macOS
~~~~~

Open Terminal and type ``screen /dev/cu.usbmodem1422 115200``, replacing
``/dev/cu.usbmodem1422`` with the port you found earlier. This will open the
micro:bit's serial output and show all messages received from the device.

To exit, press Ctrl-A then Ctrl-\\ and answer Yes to the question. There are
many ways back to a command prompt including Ctrl-A then Ctrl-D, which will
detach screen, but the serial port with still be locked, preventing other
applications from accessing it. You can then restart screen by typing
``screen -r``.

Linux
~~~~~

To connect to Debian based distributions, like Ubuntu, you might need
to add yourself to the ``dialout`` group, which grants write access to the
serial device ``/dev/ttyACM0``::

  sudo usermod -a -G dialout ${USER}

You may need to relogin in order for the changes to groups to take effect.

Using the ``screen`` program, type ``screen /dev/ttyUSB0 115200``, replacing
``/dev/ttyUSB0`` with the port you found earlier.

To exit, press ``Ctrl-A`` then ``\`` and answer ``Yes`` to the question.
There are many
ways back to a command prompt including Ctrl-A then Ctrl-D, which will detach
screen. All serial output from the micro:bit will still be received by
``screen``, the serial port will be locked, preventing other applications from
accessing it. You can restart screen by typing ``screen -r``.

Using ``picocom``, type ``picocom /dev/ttyACM0 -b 115200``, again replacing
``/dev/ttyACM0`` with the port you found earlier.

To exit, press Ctrl-A then Ctrl-Q.
