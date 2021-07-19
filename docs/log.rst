Data logging **V2**
*******************

.. py:module:: log

This module lets you log data to a ``MY_DATA`` file saved on a
micro:bit **V2**.

Further guidance on the feature can be found on the
`data logging page of the microbit.org website <https://microbit.org/get-started/user-guide/data-logging/>`_.

Functions
=========

.. py:function:: set_labels(value, timestamp=log.MILLISECONDS)

   Set up the log file header.

   Each call to this function with positional arguments will generate a new
   header entry into the log file.

   If the programme starts and a log file already exists it will compare the
   labels setup by this function call to the last headers declared in the
   file. If the headers are different it will add a new header entry at the
   end of the file.

* **value**: Positional arguments used to generate the log headers,
  which go on the first line of the CSV file. For example, ``set_labels("A","B","C")``
  will create three column headers titled ``A``, ``B`` and ``C`` in that order. 
* **timestamp**: Select the timestamp unit that will be automatically
  added as the first column in every row. Timestamp values can be one of
  ``MILLISECONDS``, ``SECONDS``, ``MINUTES``, ``HOURS``, ``DAYS`` or ``None`` to
  disable the timestamp.

.. py:function:: set_mirroring(value)

   Mirrors the datalogging activity to the serial output.
   Serial mirroring is off by default. 

* **value**: Boolean. ``True`` will enable mirroring data to the serial output.

.. py:function:: delete(full=False)

   Deletes the contents of the log, including headers.
   To add the log headers the ``set_labels`` function has to be called again
   after this.

* **full**: Selects a "full" erase format that removes the data from the
  flash storage. If set to ``False`` it uses a "fast" method,
  which invalidates the data instead of performing a slower 
  full erase.

.. py:function:: add({key:value})
                 add(key=value)
   
    There are two ways to add a data row into the log:

    1. From a positional argument dictionary (key == label)
        - e.g. log.add({ 'temp': microbit.temperature() })

    2. From keyword arguments (argument name == label)
        - e.g. log.add(temp=microbit.temperature())

    Each call to this function adds a row to the log.

    New data labels (dictionary keys or keyword arguments) not already
    specified via the `set_labels` function, or by a previous call to this
    function, will trigger a new header entry to be added to the log with
    the extra label.

    Labels previously specified and not present in this function call will be
    skipped with an empty value in the log row.

Example
=======

An example that runs through some of the functions of the log module API::

    from microbit import *
    import log

    # Creates a new "log" file with the given "headers", timestamp added by default
    log.set_labels('temperature', 'brightness')

    # Configuring a different time unit for the timestamp
    log.set_labels('temperature', 'brightness', timestamp=log.SECONDS)

    # Enables the serial mirroring
    log.set_mirroring(True)

    # Set the timer to log data every 1h20m30s50ms
    run_every(h=1, min=20, s=30, ms=50)

    while True:
        if button_a.is_pressed() and button_b.is_pressed():
            log.delete(full=True)
        elif button_a.is_pressed():
            # Records the temperature & brightness every 00:01:20:30:50 (dd:hh:mm:ss:ms).
            log.add({
            "temperature": temperature(),
            "brightness": display.read_light_level()
            })
            display.show(Image.HAPPY)
            sleep(500)
        else:
            display.show(Image.CONFUSED)
