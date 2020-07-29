.. role:: python(code)
   :language: python

Release notes
~~~~~~~~~~~~~~~~~~~~~~~~

Version 0.7
=============================

*Version 0.7 is based on the development branch v0.6, but is not compatible with v0.5 or v0.6 parsings and requires a full reparse of the blockchain.*

Notable Changes
------------------------

- Python: New fluent interface

  A new fluent Python interface allows to execute many operations (such as filtering transactions) efficiently in C++, resulting in a major performance increase.

- Parser support for CTOR (Canonical Transaction Ordering Rule)

  BlockSci's parser has been updated to support arbitrary transaction ordering rules within a block (e.g., Bitcoin Cash's canonical transaction ordering).

- Transaction Input<->Output mapping

  Inputs now reference the output they are spending, and vice versa. These can be looked up using :attr:`blocksci.Output.spending_input` and :attr:`blocksci.Input.spent_output`

- New config files to store configurations for different blockchains

  Settings for blockchain parsings are now stored in a JSON config file. Find more information in the `setup instructions`_

- Testing: Python test suite and CI have been added

  We've added a small test suite for the Python interface. It uses a special regtest blockchain created using our `testchain generator`_.
  Current test coverage is limited, and we welcome contributions to extend it.

.. _testchain generator: https://github.com/citp/testchain-generator
.. _setup instructions: https://citp.github.io/BlockSci/setup.html


Important bug fixes
----------------------

- v0.5 contains a bug that causes reused addresses to receive a new ID, rather than the previously assigned ID. If the address had been used multiple times before, subsequent occurrences would receive the old ID again, resulting in lookups that show only one transaction associated with the address. If the address had been used only once before, a lookup will miss this previous occurrence. Bitcoin parsings beyond block height 572072 are affected. You can find more information in `this document`_. (`Issue #272`_)

.. _this document: https://docs.google.com/document/d/1Tq8OLcNy9Z5rh7YeVpW8VPnYVTwXW7qWRZ4B4OwPRSw/edit?usp=sharing
.. _Issue #272: https://github.com/citp/BlockSci/issues/272

- The parser now builds the index to look up wrapping addresses. Previously, when retrieving the equiv address from a wrapped address (e.g., P2PK that is wrapped by a P2SH address), it would not include the wrapping address. (`PR #402`_)

.. _PR #402: https://github.com/citp/BlockSci/pull/402

- *v0.6 only:* incorrect handling of compressed public keys resulted in multisig addresses (:class:`blocksci.MultisigAddress`) not being correctly deduplicated (i.e. they would receive a new ID on reuse). (Fixed on 01/31/2020, `PR #367`_)

.. _PR #367: https://github.com/citp/BlockSci/pull/367


Other changes and bug fixes
-----------------------------

- Multisig addresses with invalid public keys are considered non-standard

- Upgraded dependencies (including range-v3, pybind11 and RocksDB)

- :attr:`blocksci.Address.[ins|outs|in_txes|out_txes|txes]` now return iterators

- The :class:`blocksci.heuristics.change.ChangeHeuristic` interface has been rewritten.

  - Heuristics now return an :class:`blocksci.OutputIterator` instead of a set of outputs.
  - `ChangeHeuristic.unique_change` now returns a new :class:`ChangeHeuristic` object, allowing to use it to compose with other change heuristics.
  - A new `None` heuristic has been added and is also the default heuristic for change address clustering (effectively disabling it).
  - A new `Spent` heuristic allows to refine heuristics that return unspent outputs as potential change outputs.
  - Clustering no longer performs change address clustering by default. You can still specify a change address heuristic to enable it.

- Added a new tool to the parser (``blocksci config.json doctor``) that can detect a few common issues with the setup

- Added a new tool (``blocksci_check_integrity``) that computes a hash value over the BlockSci data produced by the parser

- Correctly handle Schnorr signatures on Bitcoin Cash with a length of 65 bytes (`PR #395`_)

- Changed `in(s)/out(s)` in method/property names to `input(s)/output(s)` to avoid confusion with incoming and outgoing funds (`PR #392`_)

- Fixed an inconsistency in recording the tx that first spends a script (`PR #385`_)

- :func:`chain.cpp.filter_tx` has been removed in favor of the new fluent interface (`Issue #254`_)

- Recognize address formats that use more than one version byte (`Issue #246`_)

- The parser will detect if another instance is already running on the same data directory (`Issue #211`_)

- :func:`~blocksci.cluster.ClusterManager.create_clustering` now accepts a start and end height for clustering only a specific block range (does not apply to linking of wrapped with wrapping addresses) (`Issue #118`_)

- Fixed rounding inconsistencies for values in Zcash (`Issue #117`_)

- Added *Witness Unknown* address type support (`Issue #112`_)

- Added transaction version numbers (`Issue #92`_)


.. _Issue #92: https://github.com/citp/BlockSci/issues/92
.. _Issue #112: https://github.com/citp/BlockSci/issues/112
.. _Issue #117: https://github.com/citp/BlockSci/issues/117
.. _Issue #118: https://github.com/citp/BlockSci/issues/118
.. _Issue #211: https://github.com/citp/BlockSci/issues/211
.. _Issue #246: https://github.com/citp/BlockSci/issues/246
.. _Issue #254: https://github.com/citp/BlockSci/issues/254
.. _PR #367: https://github.com/citp/BlockSci/pull/367
.. _PR #385: https://github.com/citp/BlockSci/pull/385
.. _PR #392: https://github.com/citp/BlockSci/pull/392
.. _PR #395: https://github.com/citp/BlockSci/pull/395


Known bugs and limitations
----------------------------

- Performance of directly accessing addresses and iterators/ranges in the Python interface is slower than in v0.5 (only noticeable when accessing them in large volumes)
- Iterating over an ``AddressIterator`` in pure Python causes a segfault. Use ``.to_list()`` to retrieve a list of the results over which you can iterate.


Version 0.5.0
========================

Feature Enhancements
---------------------

- Expanded iterator and range functionality to return NumPy arrays.

  Many methods and properties of BlockSci objects return range or iterator objects such as :class:`blocksci.TxRange`. These objects allow vectorized operations over sequences of BlockSci objects. Their API matches up with the API of their member objects, and thus :class:`blocksci.TxRange` has almost the same set of methods as :class:`blocksci.Tx`. These methods will effiently call the given method over all items in the range or iterator. Depending on the return type of the method, the result will either be another range, a NumPy array, or a python list. For further information, look for these classes in the reference.

- Add custom BlockSci pickler to enable sending and recieving serialized BlockSci objects. This means that returning BlockSci objects from the multiprocessing interface now works correctly.

- Enhance the change address heuristics interface

  Change address heuristics are now composible in order to form new customized heuristics using the :class:`blocksci.heuristics.change.ChangeHeuristic` interface. These can be used in combination with the new clustering interface described below.

- Incorporate clustering module into main BlockSci library

  The formerly external clustering module is now avaiable as :mod:`blocksci.cluster`. Further, it is now possible to generate new clusterings through the python interface using the :func:`~blocksci.cluster.ClusterManager.create_clustering` method. Users can select their choice of change address heuristic in order to experiment with different clustering strategies.

- Simplified build system

  BlockSci's install process no longer requires the compilation of any external dependencies to compile on Ubuntu 16.04.

  The BlockSci library no longer has any public dependencies so compiling against it will not require linking against anything else.

  The CMake build script has now been updated to install a Config file which allows you to use `find_package(blocksci)` to import BlockSci's target's into your build script. This makes it much easier to build libraries that use BlockSci as a dependency.

  The BlockSci python module has been moved into a separate module to allow for a simple SetupTools or pip based install process: `pip install -e pyblocksci`. The main BlockSci library must be installed first for this to work.

  Finally, install instructions_ for the mac have been added along with Ubuntu 16.06 instructions.

- Updated mempool recorder and integrated it into BlockSci interface.

  For instructions on running the mempool recorder and using the data it produces, see the setup_ section.

- Improve and clean up auto generated API reference.

  All method signatures display correct types and all properties display the type of the returned value. Further, all types link to their definition in the documentation.

.. _instructions: https://citp.github.io/BlockSci/compiling.html

Bug Fixes
----------
- Fixed rare parser bug causing blocks to be ignored (`Issue #45`_, `Issue #69`_)
- Removed hardcoded paths (`Issue #72`_)
- Fixed :func:`~blocksci.Block.miner` (`Issue #76`_)
- Made it possible to iterate over the ouputs linked to a cluster (`Issue #81`_)
- Added pickling support (`Issue #84`_)
- Added :func:`~blocksci.Blockchain.reload` (`Issue #98`_)
- Fixed infinite loop in `segmentChain` (`Issue #104`_)

.. _setup: https://citp.github.io/BlockSci/setup.html
.. _Issue #45: https://github.com/citp/BlockSci/issues/45
.. _Issue #69: https://github.com/citp/BlockSci/issues/69
.. _Issue #72: https://github.com/citp/BlockSci/issues/72
.. _Issue #76: https://github.com/citp/BlockSci/issues/76
.. _Issue #81: https://github.com/citp/BlockSci/issues/81
.. _Issue #84: https://github.com/citp/BlockSci/issues/84
.. _Issue #98: https://github.com/citp/BlockSci/issues/98
.. _Issue #104: https://github.com/citp/BlockSci/issues/104

Version 0.4.5
========================

Feature Enhancements
---------------------

- Safe incremental updates

  Following an number of enhancements BlockSci is now capable of safely performing incremental updates. The AWS distribution of BlockSci now includes a Bitcoin full node and will automatically update the BlockChain once per hour. For local installations of BlockSci, see the readme for setup instructions.

- Introduced new concept of Equivalent Addresses which includes two types of equivalences, Type Equivalent and Script Equivalent. Type equivalent refers to two addresses using the same secret in a different way such as how a single pubkey could be used for a Pay to Pubkey Hash address and a Pay To Witness Pubkey Hash address. Script Equivalent refers to a Pay tp Script Hash address being equivalent to the address it contains. Address.equiv() and the EquivAddress class were added to support these concepts. See the documentation for more information.

- Enabled the opening of multiple Blockchain objects in the same notebook by removing internal usage of Singleton pattern.

- Proper handling of segwit tx and block size distinctions. This included updating the parser to store the size of each transaction excluding segwit data and as as supporting the 3 new notions of size that segwit introduced.

- Proper handling of bech32 addresses.

  - Blockchain.address_from_string() now supports lookup of bech32 addresses.

  - Address objects now display the correct human readable address depending on the address type.

- Improved initial chain parsing from 24 hours down to 12 hours and reduced in parser data size due to unification of the hash index database and parser address hash index database.


Breaking Changes
---------------------

- Updated to new data version for the parser output requiring a rerun of the blocksci_parser.

- In order to allow multiple blockchain objects. All constructors and factory methods were removed with parallel methods added to the chain object. For instance Tx(hash) is now chain.tx_with_hash(hash).

- Removed Address.script and merged its functionality into Address

- Modified Address.outs(), Address.balance(), and related functions to only return results for places on the Blockchain where that address appeared in a top level context (Not wrapped inside another address).

- Renamed various methods from using script in their name to address in order to reflect updated terminology.

- Removed ScriptType since its functionality was superseded by EquivAddress

Bug Fixes
-------------
- Fixed segwit size handling as stated above. (`Issue #43`_)
- Fixed chain.filter_txes (`Issue #50`_)
- Fixed P2SH API issued. (`Issue #53`_)

 .. _Issue #43: https://github.com/citp/BlockSci/issues/43
 .. _Issue #50: https://github.com/citp/BlockSci/issues/50
 .. _Issue #53: https://github.com/citp/BlockSci/issues/53


Version 0.3
========================

5x performance increase
-----------------------
We achieved a roughly 5x performance increase (for the C++ interface) at the cost of a small (under 4%) increase in memory consumption. BlockSci can now iterate over every transaction input and output on the Bitcoin blockchain in about 1 second on a single 4-core EC2 instance. Most of the improvements came from improving memory alignment in data files.

In our paper_ we presented performance results for iterating over transactions using the C++ library (up to block 478,449 of the Bitcoin blockchain). In the table below we compare the timings reported in the paper (Old) to the corresponding timings for version 0.3 (New).

.. _paper: https://arxiv.org/pdf/1709.02489.pdf

+-----------------------------+----------------------+----------------------------+
|Iterating over               | Single Threaded      |     Multithreaded          |
|                             +-----------+----------+-------------+--------------+
|                             | Old       |   New    |   Old       |    New       |
+-----------------------------+-----------+----------+-------------+--------------+
|Transaction headers          | 13.1 sec  | 3.0 sec  | 3.2 sec     | 0.6 sec      |
+-----------------------------+-----------+----------+-------------+--------------+
|Transaction outputs          | 27.9 sec  | 3.4 sec  | 6.6 sec     | 0.9 sec      |
+-----------------------------+-----------+----------+-------------+--------------+
|Transaction inputs & outputs | 46.4 sec  | 4.2 sec  | 10.3 sec    | 1.1 sec      |
+-----------------------------+-----------+----------+-------------+--------------+
|Headers in random order      | 303.0 sec | 99.9 sec | Unsupported |  Unsupported |
+-----------------------------+-----------+----------+-------------+--------------+

In future versions we plan to bring the Python performance for most common types of queries closer to the C++ performance figures in the table.

New data format
------------------

As noted above, we updated the data format. This requires rerunning the parser from scratch if you are running a local
copy of BlockSci. If you are using the EC2 AMI image you can simply launch a new server with the BlockSci 0.3 image.

SegWit support & API changes
-----------------------------
- We provide full support to two new address types (Pay to Witness Script Hash and Pay to Witness Pubkey Hash)
- New distinction between address type and script type

  Version 0.3 introduces a distinction between two outputs which are sent the same way and two outputs that can be spent
  using the same information. This difference comes up in multiple circumstances including when a the same public key is used
  is a pay to public key hash output and inside a multisignature output.

  Inside the BlockSci interface these two related concepts map to the Address and Script types respectively. Both objects
  possess very similar APIs, but operate somewhat differently. As an example, given a specific P2PKH address, :python:`address`, the
  then :python:`address.outs()` will return all outputs sent to that specific address. If the pubkey used in that address
  was also used in another type of output, this would not be shown. However calling :python:`address.script.outs()` will return
  all outputs where that pubkey was used in any form.

  Additionally Script objects contain a large amount of information about the script used. For instance Multisig scripts provide
  access to all the pubkeys involved and P2SH scripts provide access to the wrapped address if it is known.

- Moved heuristic-based behavior to a separate module (blocksci.heuristics) to make it easier to distinguish it from core functionality.

  The heuristics library contains two main types of heuristics: change address identification and transaction labeling.
  In the previous version these functionalities were included in the main functionality of the library making it difficult to
  distinguish between functions which are guaranteed to be correct and functions which only produce guesses.

  New versions of the API are accessable by using

  .. code-block:: python

        blocksci.heuristics.change_by_client_change_address_behavior(tx)
        blocksci.heuristics.is_coinjoin(tx)

Additional index lookup
------------------------
We have added an index to allow the lookup of transactions by hash and addresses by address string.

Transactions can be looked up via :python:`blocksci.Tx(hash_string)` and addresses can be looked up via :python:`blocksci.Address.from_string(address_string)`.

Bug fixes
---------------------
 - Many causes of crashes and instability have now been resolved.
 - Segwit support has been introduced. (`Issue #1`_)
 - The address index lookups now will return correct results. (`Issue #6`_)
 - The parser no longer reads beyond memory boundaries causing occasional crashes. (`Issue #9`_)
 - The initial header parse phase is now multithreaded leading to a substantial performance increase. (`Issue #12`_)
 - Fixed bitcoin-api-cpp headers so it now works on both mac and linux. (`Issue #15`_)
 - The parser now provides feedback as it goes. (`Issue #26`_)
 - The python module no longer crashes on exit. (`Issue #25`_)

 .. _Issue #1: https://github.com/citp/BlockSci/issues/1
 .. _Issue #6: https://github.com/citp/BlockSci/issues/6
 .. _Issue #9: https://github.com/citp/BlockSci/issues/9
 .. _Issue #12: https://github.com/citp/BlockSci/issues/12
 .. _Issue #15: https://github.com/citp/BlockSci/issues/15
 .. _Issue #25: https://github.com/citp/BlockSci/issues/25
 .. _Issue #26: https://github.com/citp/BlockSci/issues/26

Limitations
-------------------
Incremental updating of the blockchain is currently not supported due to some continuing bugs in blockchain reorg handling.
Rerunning the parser in the uncommon situation that a previously parsed block has been orphaned may cause data corruption.

Version 0.2
========================

This version was the initial release of BlockSci. Documentation_ for version 0.2 is still available.

.. _Documentation: https://citp.github.io/BlockSci/0.2/
