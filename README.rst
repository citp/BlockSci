BlockSci
~~~~~~~~~~~~~~~~~~

The Bitcoin blockchain — currently 170 GB and growing — contains a massive amount of data that can give us insights into the Bitcoin ecosystem, including how users, businesses, and miners operate. BlockSci enables fast and expressive analysis of Bitcoin’s and many other blockchains. The accompanying working paper explains its design and applications: https://arxiv.org/pdf/1709.02489.pdf

Current tools for blockchain analysis depend on general-purpose databases that provide "ACID" guarantees. But that’s unnecessary for blockchain analysis where the data structures are append-only. We take advantage of this observation in the design of our custom in-memory blockchain database as well as an analysis library. BlockSci’s core infrastructure is written in C++ and optimized for speed. (For example, traversing every transaction input and output on the Bitcoin blockchain takes only **1 second** on our r4.2xlarge EC2 machine.) To make analysis more convenient, we provide Python bindings and a Jupyter notebook interface. 

Documentation
=====================
Documentation_ is available for the Python interface.

.. _Documentation: https://citp.github.io/BlockSci/

Additionally, a demonstration Notebook_ is available in the Notebooks folder.

.. _Notebook: https://citp.github.io/BlockSci/demo.html

For installation instructions, see below. 

Latest release (BlockSci v0.5.0)
================================

Version 0.5.0 focuses mainly on improvements and cleanups in the Python interface. The largest new feature is the introduction of vectorized operations which return NumPy arrays, enabling much more rapid usage of BlockSci's Python interface. You can read more details about the release in the `release notes`_. We are releasing a new AMI_ running 0.5.0 (explained under "Quick setup" below).

.. _release notes: https://citp.github.io/BlockSci/changelog.html#version-0-5-0
.. _AMI: https://console.aws.amazon.com/ec2/home?region=us-east-1#launchAmi=ami-0d0091e593d44cce1


Quick setup using Amazon EC2
==============================

AMI Last updated on May 8, 2018.

If you want to start using BlockSci immediately, we have made available an EC2 image: ami-0d0091e593d44cce1_. We recommend using an instance with 60 GB of memory or more for optimal performance (r5.2xlarge). As of August 2019 the default disk size of 500GB may not suffice anymore, we therefore recommend choosing a larger disk size (e.g., 600 GB) when you first create the instance. On boot, a Jupyter Notebook running BlockSci will launch immediately. To access the notebook, you must set up port forwarding to your computer. Inserting the name of your private key file and the domain of your ec2 instance into the following command will make the Notebook available on your machine.

.. code-block:: bash

	ssh -i .ssh/your_private_key.pem -N -L 8888:localhost:8888 ubuntu@your_url.amazonaws.com

This sets up an SSH tunnel between port 8888 on your remote EC2 instance and port 8888 on your localhost. You can use whichever port you like on your local machine. Next, you can navigate to http://localhost:8888/ in your browser and log in with the password 'blocksci'. A demo notebook will be available for you to run and you can begin exploring the blockchain. Don't forget to shut down the EC2 instance when you are finished since EC2 charges hourly.

AWS instances suffer from a `known performance issue`_ when starting up from an existing AMI. When the machine starts up it doesn't actually load all of the data on the disk so that startup can be instant. Instead it only loads the data when it is accessed for the first time. Thus BlockSci will temporarily operate slowly when the image has first been launched. Within about 20 minutes after launch, the most crucial data files will be loaded to disk from the network, and most queries should run at full speed, including all examples in the demo Notebook. After about 3.5 hours, all data will be loaded to disk and all queries will reach full speed.

There is no need for user intervention to resolve this issue since the machine will do so automatically on launch.

The AMI contains a fully updated version of the Bitcoin blockchain as of the creation date of the AMI (May 8, 2018). Additionally it will automatically start a Bitcoin full node and update the blockchain once every hour to the latest version of the chain.
When you start the AMI for the first time, it will take a few hours for the full node to synchronize with the Bitcoin network and for new blocks (after May 2018) to become available in BlockSci.

.. _ami-0d0091e593d44cce1: https://console.aws.amazon.com/ec2/home?region=us-east-1#launchAmi=ami-0d0091e593d44cce1
.. _known performance issue: https://docs.aws.amazon.com/AWSEC2/latest/UserGuide/ebs-initialize.html

Using the analysis library
============================

After the parser has been run, the analysis library is ready for use. This can again be used through two different interfaces

C++
------

In order to use the C++ library, you must compile your code against the BlockSci dynamic library and add its headers to your include path. The Blockchain can then be constructed given the path to the output of the parser.

.. code-block:: c++

	#include <blocksci/blocksci.hpp>
	
	int main(int argc, const char * argv[]) {
    		blocksci::Blockchain chain{"file_path_to_output-directory"};
	}

Python
-------

Note that BlockSci only supports Python 3.

To use the BlockSci in Python, you only need to import the BlockSci library. By default the library is installed into BlockSci/Notebooks. To use the library first open the Python interpreter in that folder:

.. code-block:: bash

	cd BlockSci/Notebooks
	python3
	
With the Python interpreter open, the following code will load a Blockchain object created from the data output by the parser:

.. code-block:: python

	import blocksci
	chain = blocksci.Blockchain("file_path_to_parser_output-directory")

If you would like to use BlockSci through a web interface, we recommend the use of `Jupyter Notebook`_. Once Jupyter is installed, simply navigate into BlockSci/Notebooks and run:

.. code-block:: bash

	jupyter notebook
	
which will open a window in your browser to the Jupyter server.

.. _Jupyter Notebook: https://jupyter.readthedocs.io/en/latest/install.html


Setting up BlockSci Locally
======================================

Compilation_ instructions as well as setup_ instructions are available in the documentation.

.. _Compilation: https://citp.github.io/BlockSci/compiling.html
.. _setup: https://citp.github.io/BlockSci/setup.html


Getting help
============

Please make sure to check the list of `Frequently Asked Questions`_ first.
If you've encountered a bug or have a question about using BlockSci not answered in the FAQ, the best way to get help is to open a GitHub issue. We are an academic team and aren't able to provide the standard of support that you might expect for a commercial project, but we'll do our best. 

.. _Frequently Asked Questions: https://github.com/citp/BlockSci/wiki


Contributing
============

We highly welcome contributions to BlockSci. Below we've listed a few ways you can help improve BlockSci:

- *Maintenance:* We greatly appreciate help in maintaining BlockSci, including raising issues with reproducible examples, reviewing pull requests, helping answer questions about using BlockSci, or fixing smaller bugs.
- *Documentation:* We welcome contributions that improve our documentation_ and FAQ_ or add helpful comments to the code.
- *Testing:* We welcome contributions that extend or improve our existing Python test suite. We also welcome improvements of the `testchain-generator`_ that we use to generate a synthetic blockchain to run tests against.
- *Code contributions:* If you're interested in making larger code contributions (e.g., adding new features, extensive rewrites of existing code), please contact us first.

We're currently working on a new version on the `v0.6 branch`_. Most contributions should use this development branch as a starting point. (The development branch can be unstable at times. The master branch contains the last stable version for which an AMI was released. All other branches are feature branches that shouldn't be used.)

.. _testchain-generator: https://github.com/citp/testchain-generator
.. _v0.6 branch: https://github.com/citp/BlockSci/tree/v0.6
.. _documentation: https://citp.github.io/BlockSci/
.. _FAQ: https://github.com/citp/BlockSci/wiki

Team & contact info
===================

BlockSci was created by Harry Kalodner, Steven Goldfeder, Alishah Chator, Malte Möser, and Arvind Narayanan at Princeton University. It is supported by NSF grants CNS-1421689 and CNS-1651938 and an NSF Graduate Research Fellowship under grant number DGE-1148900. We've released a paper_ describing BlockSci's design and a few applications that illustrate its capabilities. You can contact the team at blocksci@lists.cs.princeton.edu.

.. _paper: https://arxiv.org/abs/1709.02489
