.. _equiv-addresses-label:

Equivalent Addresses
~~~~~~~~~~~~~~~~~~~~~~~~

Equivalent addresses reflect BlockSci's understanding that various distinct address types actually require a common piece of information to spend them. Pay to pubkey, pay to pubkeyhash and pay to witness pubkey hash all require knowledge of a given pubkey in order to be spent and can thus be considered equivalent. Similarly pay to scripthash and pay to witness scripthash both require knowledge of a given spending script.

While the `Address` type views these as distinct addresses, the `EquivAddress` type views them as the same. The API's of the two classes are highly similar, but differ in what outputs they consider as sent to them and thus for example, may display difference balances.

In other analysis tools, this distinction between `Address` and `EquivAddress` is not explicitly handled. Frequently Pay to Pubkey and Pay to Pubkey hash addresses using the same public key are treated as the same address and in many block explorers the balance of an 'address' is the sum of the balances of both types. However a Pay to witness pubkey hash address using the same pubkey is treated as completely distinct from the other addresses. BlockSci allows the user to decide which address concept they would like to work with.

Equiv Address Type
====================================

.. autoclass:: blocksci.equiv_address_type
   :members:
   :inherited-members:
   :undoc-members:

EquivAddress
====================================

.. autoclass:: blocksci.EquivAddress
   :members:
