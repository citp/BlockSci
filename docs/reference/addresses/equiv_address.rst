.. _equiv-addresses-label:

Equivalent Addresses
~~~~~~~~~~~~~~~~~~~~~~~~

Equivalent addresses reflect BlockSci's understanding that various distinct addresses actually require a common piece of information to spend them. 

BlockSci supports two distinct notions of address equivalence.

1. Type Equivalence - Addresses which require the exact same knowledge to spend them can be considered equivalent. Pay to pubkey, pay to pubkeyhash and pay to witness pubkey hash all require knowledge of a given pubkey in order to be spent and can thus be considered equivalent. Similarly pay to scripthash and pay to witness scripthash both require knowledge of a given spending script.

2. Script Equivalence - A Pay to script hash address and the address that is wrapped inside it can be considered equivalent addresses since they reflect the same piece of information.

Type Equivalence and Script Equivalence together or Type Equivalence alone form an equivalence class over the addresses. This technique allows users to find a set of addresses that is guarenteed to be controlled by the same person.

EquivAddress provides a very similar interface to address, except operates on a set of equivalent addresses rather than a single address.

In other analysis tools, this distinction between `Address` and `EquivAddress` is not explicitly handled. Frequently Pay to Pubkey and Pay to Pubkey hash addresses using the same public key are treated as the same address and in many block explorers the balance of an 'address' is the sum of the balances of both types. However a Pay to witness pubkey hash address using the same pubkey is treated as completely distinct from the other addresses. Further when a pubkey is used both in P2PKH addresses as well as wrapped inside a P2SH, these are frequently treated as different addresses.  BlockSci allows the user to decide which address concept they would like to work with.

EquivAddress
====================================

.. autoclass:: blocksci.EquivAddress
   :members:
