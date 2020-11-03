Change Address Heuristics
--------------------------

BlockSci supports a number of different heuristics for determining potential change outputs for a given transaction.
The heuristics are sometimes contradictory and thus we provide users with the ability to choose which heuristics they wish to apply.
Further, you can combine different change address heuristics through the various composition operators of the :py:class:`blocksci.heuristics.change.ChangeHeuristic` class.

.. autoclass:: blocksci.heuristics.change
    :members:
    :special-members: __call__, __and__, __or__, __sub__

Note that most heuristics can return multiple outputs as candidates. To only return an output when there's only a single candidate, use `.unique_change`.

Static Heuristics
~~~~~~~~~~~~~~~~~

Static heuristics do not depend on the outputs being spent.

.. autoattribute:: blocksci.heuristics.change.address_reuse

    If input addresses appear as an output address, the client might have reused addresses for change.

.. autoattribute:: blocksci.heuristics.change.address_type

    If all inputs are of one address type (e.g., P2PKH or P2SH), it is likely that the change output has the same type.

.. autoattribute:: blocksci.heuristics.change.optimal_change

    If there exists an output that is smaller than any of the inputs it is likely the change. If a change output was larger than the smallest input, then the coin selection algorithm wouldn't need to add the input in the first place.

.. autofunction:: blocksci.heuristics.change.power_of_ten_value

    Detects possible change outputs by excluding output values that are multiples of 10^`digits`, as such values are unlikely to occur randomly.

.. autoattribute:: blocksci.heuristics.change.client_change_address_behavior

    Most clients will generate a fresh address for the change. If an output is the first to send value to an address, it is potentially the change.

.. autoattribute:: blocksci.heuristics.change.legacy

    The original change address heuristic that was the default used in BlockSci before version 0.6.


Dynamic Heuristics
~~~~~~~~~~~~~~~~~~

Dynamic heuristics depend on the state of the blockchain. If outputs of a transaction are spent, the results returned by the following heuristics can change.

.. autoattribute:: blocksci.heuristics.change.locktime

    Bitcoin Core sets the locktime to the current block height to prevent fee sniping. If an output has been spent and it matches this transaction's locktime behavior, it is possibly the change output.
    (This heuristic will return unspent outputs as potential candidates.)

.. autoattribute:: blocksci.heuristics.change.peeling_chain

    If the transaction is a peeling chain, returns the outputs that continue the peeling chain.
    (This heuristic will return unspent outputs as potential candidates.)


Utility Heuristics
~~~~~~~~~~~~~~~~~~

.. autoattribute:: blocksci.heuristics.change.none

    This heuristics will never return any outputs, allowing it to be used to disable clustering based on change addresses.

.. autoattribute:: blocksci.heuristics.change.spent

    Returns outputs that have been spent. Useful to exclude unspent outputs from dynamic heuristics that return unspent outputs as potential change outputs.


Using Composition Operators
~~~~~~~~~~~~~~~~~~~~~~~~~~~

You can use the union (`|`), intersection (`&`) and difference (`-`) operators to combine existing change address heuristics into new ones. This also works with `unique_change`.
For example, the following heuristic would return a single change output only if the address reuse and the optimal change heuristics agree on a unique change output:

.. code-block:: python

    blocksci.heuristics.change.address_reuse.unique_change & blocksci.heuristics.change.optimal_change.unique_change


Similarly, if you want to remove unspent outputs from the output candidates returned by the :attr:`blocksci.heuristics.change.locktime` heuristics, you can do the following:

.. code-block:: python

    blocksci.heuristics.change.locktime & blocksci.heuristics.change.spent
