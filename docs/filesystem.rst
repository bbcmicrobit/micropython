Local Persistent File System
****************************

It is useful to store data in a persistent manner so that it remains intact
between restarts of the device. On traditional computers this is often achieved
by a file system consisting of named files that hold raw data, and named
directories that contain files. Python supports the various operations needed
to work with such file systems.

However, since the micro:bit is a limited device in terms of both hardware and
storage capacity MicroPython provides a small subset of the functions needed
to persist data on the device. Because of memory constraints **there is
approximately 30k of storage available** on the file system.

.. warning::

    Re-flashing the device will DESTROY YOUR DATA.

    Since the file system is stored in the micro:bit's flash memory and
    flashing the device rewrites all the available flash memory then all your
    data will be lost if you flash your device.

    However, if you switch your device off the data will remain intact until
    you either delete it (see below) or re-flash the device.

MicroPython on the micro:bit provides a flat file system; i.e. there is no
notion of a directory hierarchy, the file system is just a list of named
files. Reading and writing a file is achieved via the standard Python ``open``
function and the resulting file-like object (representing the file) of types
``TextIO`` or ``BytesIO``. Operations for working with files on the file system
(for example, listing or deleting files) are contained within the
:py:mod:`os` module.

If a file ends in the ``.py`` file extension then it can be imported. For
example, a file named ``hello.py`` can be imported like this: ``import hello``.

An example session in the MicroPython REPL may look something like this::

    >>> with open('hello.py', 'w') as hello:
    ...    hello.write("print('Hello')")
    ...
    >>> import hello
    Hello
    >>> with open('hello.py') as hello:
    ...   print(hello.read())
    ...
    print('Hello')
    >>> import os
    >>> os.listdir()
    ['hello.py']
    >>> os.remove('hello.py')
    >>> os.listdir()
    []

.. py:function:: open(filename, mode='r')

    Returns a file object representing the file named in the argument
    ``filename``. The mode defaults to ``'r'`` which means open for reading in
    text mode. The other common mode is ``'w'`` for writing (overwriting the
    content of the file if it already exists). Two other modes are available
    to be used in conjunction with the ones describes above: ``'t'`` means
    text mode (for reading and writing strings) and ``'b'`` means binary mode
    (for reading and writing bytes). If these are not specified then ``'t'``
    (text mode) is assumed. When in text mode the file object will be an
    instance of ``TextIO``. When in binary mode the file object will be an
    instance of ``BytesIO``. For example, use ``'rb'`` to read binary data from
    a file.


.. py:class::
    TextIO
    BytesIO

    Instances of these classes represent files in the micro:bit's flat file
    system. The TextIO class is used to represent text files. The BytesIO
    class is used to represent binary files. They work in exactly the same
    except that TextIO works with strings and BytesIO works with bytes.

    You do not directly instantiate these classes. Rather, an appropriately
    configured instance of the class is returned by the ``open`` function
    described above.

    .. py:method:: close()

        Flush and close the file. This method has no effect if the file is
        already closed. Once the file is closed, any operation on the file
        (e.g. reading or writing) will raise an exception.

    .. py:method:: name()

        Returns the name of the file the object represents. This will be the
        same as the ``filename`` argument passed into the call to the ``open``
        function that instantiated the object.

    .. py:method:: read(size)

        Read and return at most ``size`` characters as a single string or
        ``size`` bytes from the file. As a convenience, if ``size`` is
        unspecified or -1, all the data contained in the file is returned.
        Fewer than ``size`` characters or bytes may be returned if there are
        less than ``size`` characters or bytes remaining to be read from
        the file.

        If 0 characters or bytes are returned, and ``size`` was not 0, this
        indicates end of file.

        A ``MemoryError`` exception will occur if ``size`` is larger than the
        available RAM.

    .. py:method:: readinto(buf, n=-1)

        Read characters or bytes into the buffer ``buf``. If ``n`` is supplied,
        read ``n`` number of bytes or characters into the buffer ``buf``.

    .. py:method:: readline(size)

        Read and return one line from the file. If ``size`` is specified, at
        most ``size`` characters will be read.

        The line terminator is always ``'\n'`` for strings or ``b'\n'`` for
        bytes.

    .. py:method:: writable()

        Return ``True`` if the file supports writing. If ``False``, ``write()``
        will raise ``OSError``.

    .. py:method:: write(buf)

        Write the string or bytes ``buf`` to the file and return the number of
        characters or bytes written.
