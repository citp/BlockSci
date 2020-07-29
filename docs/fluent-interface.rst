Fluent Interface (Python)
==========================

With currently more than 430 million transactions in the Bitcoin blockchain, analyzing large parts of the blockchain has become largely infeasible using BlockSci's Python interface.
With the new version of BlockSci we are providing a new interface that allows for fast selection and filtering of blockchain data.

The new interface relies on the execution of queries in C++ for significantly better performance.
Rather than iterating over transactions in Python, the new interface allows the analyst to specify complex queries that are executed in C++.

Take the anomalous fee query from the BlockSci paper as an example:

..  code-block:: python

    [tx for block in chain for tx in block if tx.fee > 1e7]


This query evaluates every transaction on the blockchain and selects those with a fee larger than 0.1 BTC. It does so by instantiating Python objects for every single transaction and retrieving the fee for each of them.

Let's see how we can express this query using the new interface:

..  code-block:: python

    chain.blocks.txes.where(lambda tx: tx.fee > int(1e7))


Here, ``chain.blocks.txes`` selects all transactions in the blockchain. Then, the `where` clause allows to filter those transactions using the lambda syntax.
``where`` is only one of a variety of functions that can be applied to objects in the blockchain.

Using ``.select``, we can retrieve a specific property for each object:


..  code-block:: python

    chain.blocks.select(lambda b: b.txes.size)


The above example returns the number of transactions in each block.


Available Functions
--------------------------

======================= =============================================================================
Command                 Use
======================= =============================================================================
``.select()/.map()``    Selecting properties of objects
``.where()``            Filtering
``.max()``              Returns the largest value of a property
``.min()``              Returns the smallest value of a property
``.any()``              Evaluates a boolean expression and returns whether it holds for any item
``.all()``              Evaluates a boolean expression and returns whether it holds for all items
``.group_by()``         Aggregates properties based on another attribute
``.count/.size``        Returns the number of items
``.to_list()``          Returns a list of all items
======================= =============================================================================


Select
--------------------------

Count the number of transactions in every block (``.select`` and ``.map`` have the exact same functionality):

..  code-block:: python

    chain.blocks.select(lambda b: b.txes.size)


Filter
--------------------------

Select all transactions with a transaction fee greater than 0.1 BTC.

..  code-block:: python

    chain.blocks.txes.where(lambda tx: tx.fee > 10**7))


Min and Max
--------------------------

Find the transaction with the largest fee in the blockchain:

..  code-block:: python

    chain.blocks.txes.max(lambda tx: tx.fee)


Retrieve the largest fee in every block of the blockchain:

..  code-block:: python

    chain.blocks.map(lambda b: b.txes.max(lambda tx: tx.fee).map(lambda tx: tx.fee).or_value(0))


Finding the block that includes the largest fee in the blockchain:

..  code-block:: python

    chain.blocks.max(lambda b: b.txes.max(lambda tx: tx.fee).map(lambda tx: tx.fee).or_value(0))


Aggregation
--------------------------

Aggregate the value sent to addresses in a block:

..  code-block:: python

    chain.blocks[-1].outputs.group_by(lambda o1: o1.address, lambda o2: o2.value.sum)


Aggregating the value stored in different types of addresses:

..  code-block:: python

    chain.blocks[-1].outputs.where(lambda o: ~o.is_spent).group_by(
      lambda output: output.address.type,
      lambda outputs: outputs.value.sum
    )


Working with Optionals
--------------------------

Some fields return optional properties. For example, an unspent output does not have a spending transaction. Using ``.or_value``, we can provide a default value.

Get the value of all outputs in a block that have been spent after 4320 blocks or more.

..  code-block:: python

    chain.blocks[-1].map(lambda b: b.txes.outputs.where(
      lambda o: o.spending_input.map(lambda i: i.age).or_value(0) > 4320)).value.sum
    )


Equality, Comparison and Arithmetic
----------------------------------------------------

Most attributes support equality (`==, !=`) and comparison operations (`>, >=, <=, <`). When working with attributes that are represented by integers (e.g., transaction fees) the arithmetic operators `+, -, *, //, %` are supported. Note that only integer division can be used.

..  code-block:: python

    chain.blocks.map(lambda b: b.fee // b.size_bytes)


..  code-block:: python

    chain.blocks.txes.map(lambda tx: tx.input_value - tx.output_value)


Limitations
--------------------------

There are a few limitations you need to be aware of when using the interface.

- The fluent interface does not work with Python's logical boolean operators. Instead of `and`, `or` and `not` you'll need to use `&`, `|` and `~`.
- The fluent interface does not support floating point computations (see section "Equality, Comparison and Arithmetic" above).


