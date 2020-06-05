Other Utility Classes
====================================


Exchange Rates
---------------

We provide a utility class that retrieves exchange rates from Coindesk's `Bitcoin Price Index`_ and provides conversions between bitcoin and your currency of choice.

.. _Bitcoin Price Index: https://www.coindesk.com/price/bitcoin

.. autoclass:: blocksci.currency.CurrencyConverter
    :members:
    :undoc-members:
    :exclude-members: COINDESK_START, min_start, max_end, validate_date


Custom Pickler
---------------

BlockSci objects cannot be pickled by Python's standard pickle library.
Instead, we provide a custom pickler for BlockSci objects that allows to serialize them.

.. autoclass:: blocksci.pickler.Pickler

.. autoclass:: blocksci.pickler.Unpickler


Ranges and Iterators
--------------------

This section contains various ranges and iterators not described elsewhere.

.. toctree::
   :maxdepth: 2

   ranges/ranges
   iterators/iterators
