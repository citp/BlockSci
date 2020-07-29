BlockSci
~~~~~~~~~~~~~~~~~~

The Bitcoin blockchain — currently 280 GB and growing — contains a massive amount of data that can give us insights into the Bitcoin ecosystem, including how users, businesses, and miners operate. BlockSci enables fast and expressive analysis of Bitcoin’s and many other blockchains. The accompanying paper explains its design and applications: https://arxiv.org/pdf/1709.02489.pdf (an updated version of the paper has been accepted at Usenix Security and will be available soon).

Current tools for blockchain analysis depend on general-purpose databases that provide "ACID" guarantees. But that’s unnecessary for blockchain analysis where the data structures are append-only. We take advantage of this observation in the design of our custom in-memory blockchain database as well as an analysis library. BlockSci’s core infrastructure is written in C++ and optimized for speed. (For example, traversing every transaction input and output on the Bitcoin blockchain takes only **1 second** on our r5.4xlarge EC2 machine.) To make analysis more convenient, we provide Python bindings and a Jupyter notebook interface.

*This repository contains research code that may contain bugs and errors and may break in unexpected ways. We are an academic team and aren’t able to provide the level of QA and support that you might expect from a commercial project.*


Documentation
=====================

We provide instructions in our online documentation_:

- `Installation instructions`_

- `Using BlockSci`_

- `Guide for the fluent interface`_

- `Module reference for the Python interface`_

- `Troubleshooting`_

.. _documentation: https://citp.github.io/BlockSci/

.. _Installation instructions: https://citp.github.io/BlockSci/setup.html

.. _Guide for the fluent interface: https://citp.github.io/BlockSci/fluent-interface.html

.. _Using BlockSci: https://citp.github.io/BlockSci/using-blocksci.html

.. _Module reference for the Python interface: https://citp.github.io/BlockSci/reference/reference.html

.. _Troubleshooting: https://citp.github.io/BlockSci/troubleshooting.html


Our FAQ_ contains additional useful examples and tips.

.. _FAQ: https://github.com/citp/BlockSci/wiki


Latest release (BlockSci v0.7.0)
================================

Version 0.7.0 is based on the development branch 0.6, but requires a full reparse.

Version 0.7.0 comes with a new `fluent Python interface`_ for fast and expressive blockchain queries. It contains a number of `important bug fixes`_ as well as many smaller improvements. We recommend upgrading to the latest version of BlockSci and no longer use v0.5.

.. _important bug fixes: https://citp.github.io/BlockSci/changelog.html
.. _fluent Python interface: https://citp.github.io/BlockSci/fluent-interface.html

Amazon EC2 AMI
==============================

We currently do not provide an AMI for BlockSci.


Getting help
============

Please make sure to check the list of `Frequently Asked Questions`_ as well as the issue tracker first.
If you've encountered a bug or have a question about using BlockSci not already answered, the best way to get help is to open a GitHub issue. We are an academic team and aren't able to provide the standard of support that you might expect for a commercial project.

.. _Frequently Asked Questions: https://github.com/citp/BlockSci/wiki


Team & contact info
===================

BlockSci was created by Harry Kalodner, Malte Möser, Kevin Lee, Steven Goldfeder, Martin Plattner, Alishah Chator, and Arvind Narayanan at Princeton University. It is supported by NSF grants CNS-1421689 and CNS-1651938, an NSF Graduate Research Fellowship under grant number DGE-1148900 and a grant from the Ripple University Blockchain Research Initiative. We've released a paper_ describing BlockSci's design and a few applications that illustrate its capabilities (an updated version of the paper has been accepted at Usenix Security and will be available soon). You can contact the team at blocksci@lists.cs.princeton.edu.

.. _paper: https://arxiv.org/abs/1709.02489
