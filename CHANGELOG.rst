.. role:: python(code)
   :language: python

Release notes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
