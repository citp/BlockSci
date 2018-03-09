Addresses
~~~~~~~~~~~~~~~~~

Addresses in BlockSci represent a unique identifier for a particular target of a transaction output. All outputs with the same address use the exact same output script. BlockSci supports 7 styles of addreses that originate in the Bitcoin blockchain and which are also used in other blockchains that use Bitcoin's scripting language. An eight address type, nonstandard, operates as a catch-all for addresses that do not fit any other category.


Address Type
====================================

.. autoclass:: blocksci.address_type
   :members:
   :inherited-members:
   :undoc-members:


Address
====================================

.. autoclass:: blocksci.Address
   :special-members: __init__
   :members: