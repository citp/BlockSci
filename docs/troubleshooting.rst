Troubleshooting
=====================

Frequently asked questions (FAQ)
---------------------------------

We maintain a list of answers to `Frequently Asked Questions`_ on GitHub. Please make sure to check these first.

.. _Frequently Asked Questions: https://github.com/citp/BlockSci/wiki


Technical and practical questions
-----------------------------------

If you've encountered an issue or have a question about using BlockSci not answered in the FAQ, the best way to find help is to search for similar issues in our `GitHub issue tracker`_, or open a new issue.
Please make sure to provide as much helpful information as possible, including the information requested in the GitHub issue template.

Please know that we are an academic team and aren't able to provide the standard of support that you might expect from a commercial project.


.. _GitHub issue tracker: https://github.com/citp/blocksci/issues


Cryptocurrency support and data correctness
--------------------------------------------

We are not able to keep track of the development of the various blockchains that BlockSci can potentially support.
When cryptocurrencies implement new features (e.g., script types, signature schemes), there's a possibility that it breaks BlockSci in unexpected ways or requires updates to the code base (e.g., the parsing logic).
We therefore strongly recommend that you put procedures in place that may help detect issues with data validity and correctness, such as comparing transaction and address data against blockchain explorers.


Parser diagnostics
--------------------

The parser includes a diagnostic tool that allows to check for common issues with its setup.

..  code-block:: bash

    blocksci_parser <path/to/config.json> doctor
