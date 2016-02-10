Random Number Generation
************************

.. py:module:: random

This module is based upon the ``random`` module in the Python standard library.
It contains functions for generating random behaviour.

To access this module you need to::

    import random

We assume you have done this for the examples below.

Functions
=========

.. py:function:: getrandbits(n)

    Returns an integer with ``n`` random bits.

.. warning::

    Because the underlying generator function returns at most 30 bits, ``n``
    may only be a value between 1-30 (inclusive).

.. py:function:: seed(n)

    Initialize the random number generator with a known integer ``n``. This
    will give you reproducibly deterministic randomness from a given starting
    state (``n``).


.. py:function:: randint(a, b)

    Return a random integer ``N`` such that ``a <= N <= b``. Alias for
    ``randrange(a, b+1)``.


.. py:function:: randrange(stop)

    Return a randomly selected integer between zero and up to (but not
    including) ``stop``.

.. py:function:: randrange(start, stop)

    Return a randomly selected integer from ``range(start, stop)``.

.. py:function:: randrange(start, stop, step)

    Return a randomly selected element from ``range(start, stop, step)``.


.. py:function:: choice(seq)

    Return a random element from the non-empty sequence ``seq``. If ``seq`` is
    empty, raises ``IndexError``.


.. py:function:: random()

    Return the next random floating point number in the range [0.0, 1.0)


.. py:function:: uniform(a, b)

    Return a random floating point number ``N`` such that ``a <= N <= b``
    for ``a <= b`` and ``b <= N <= a`` for ``b < a``.
