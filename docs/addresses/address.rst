
Addresses
~~~~~~~~~~~~~~~~~

Addresses in BlockSci represent a unique identifier for a particular target of a transaction output. All outputs with the same address use the exact same output script. BlockSci supports 7 styles of addreses that originate in the Bitcoin blockchain and which are also used in other blockchains that use Bitcoin's scripting language. An eight address type, nonstandard, operates as a catch-all for addresses that do not fit any other category.

Address
====================================

.. autoclass:: blocksci.Address
   :members:

Pay to pubkey Address
====================================

.. autoclass:: blocksci.PubkeyAddress
   :members:
   :show-inheritance:

Pay to pubkey hash Address
====================================

.. autoclass:: blocksci.PubkeyHashAddress
   :members:
   :show-inheritance:


Pay to witness pubkey hash Address
====================================

.. autoclass:: blocksci.WitnessPubkeyHashAddress
   :members:
   :show-inheritance:


Multisignature Address
====================================

.. autoclass:: blocksci.MultisigAddress
   :members:
   :show-inheritance:

Pay to script hash Address
====================================

.. autoclass:: blocksci.ScriptHashAddress
   :members:
   :show-inheritance:

Pay to witness script hash Address
====================================

.. autoclass:: blocksci.WitnessScriptHashAddress
   :members:
   :show-inheritance:

Non-standard Address
====================================

.. autoclass:: blocksci.NonStandardAddress
   :members:
   :show-inheritance:

Op Return
====================================

.. autoclass:: blocksci.OpReturn
   :members:
   :show-inheritance:
