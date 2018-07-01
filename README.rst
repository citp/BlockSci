BlockSci
~~~~~~~~~~~~~~~~~~

This fork of BlockSci is a modified code based on the initial release, specifically tailored for Zcash and the additinal features it uses. We will continue supporting this release, implementing the new features in Zcash (sapling update) while also considering the updates to the original codebase. The accompanying working paper, where we used this tool is found at: https://cryptolux.org/images/d/d9/Zcash.pdf

Documentation
=====================
Documentation_ is available for the python interface library (the link directs to the documentation of BlockSci v0.2, as this is a fork of that release).

.. _Documentation: https://citp.github.io/BlockSci/0.2/index.html

Additionally, a demonstration Notebook_ is available in the Notebooks folder.

.. _Notebook: https://citp.github.io/BlockSci/0.2/demo.html

An example python3 code is available in the Notebooks folder as well, showing the new Zcash specific features.

For installation instructions, see below.

Local Setup
=====================
To run BlockSci locally, you must be running a full node (zcashd or an altcoin node, ran with txindex=1 as a default setting) since BlockSci requires the full serialized blockchain data structure which full nodes produce. 

Parser
----------

The next step is parsing the blockchain downloaded by the full node. The blockchain_parser binary which is built from this repo performs the transformation of this blockchain data into the customized BlockSci database format.

Currently only the RPC mode is usable. It uses the RPC interface of a cryptocurrency to extract data regarding the blockchain. We haven't tested it yet, but it should work with the forks of Zcash as well.

..  code-block:: bash

	blocksci_parser --username [user] --password [pass] --address [ip] --port [port] --output-directory zcash-data

For testing purposes an optional variable is available, so that one does not have to parse in the entire chain to test the tool.

..  code-block:: bash

	blocksci_parser --username [user] --password [pass] --address [ip] --port [port] --output-directory zcash-data --max-block [block number]

Using the Analysis Library
============================

After the parser has been run, the analysis library is ready for use. This can again be used through two different interfaces

C++
------

In order to use the C++ library, you must compile your code against the BlockSci dynamic library and add its headers to your include path. The Blockchain can then be constructed given the path to the output of the parser.

.. code-block:: c++

	#include <blocksci/chain/blockchain.hpp>
	
	int main(int argc, const char * argv[]) {
    		blocksci::Blockchain chain{"file_path_to_output-directory"};
	}

Python
-------

To use the BlockSci in python, you only need to import the BlockSci library. By default the library is installed into BlockSci/Notebooks. To use the library first open the Python interpreter in that folder:

.. code-block:: bash

	cd BlockSci/Notebooks
	python3
	
With the python interpretter open, the following code will load a Blockchain object created from the data output by the parser:

.. code-block:: python

	import blocksci
	chain = blocksci.Blockchain("file_path_to_parser_output-directory")

If you would like to use BlockSci through a web interface, we recommend the use of `Jupyter Notebook`_. Once Jupyter is installed, simply navigate into BlockSci/Notebooks and run:

.. code-block:: bash

	jupyter notebook
	
which will open a window in your browser to the Jupyter server.

.. _Jupyter Notebook: https://jupyter.readthedocs.io/en/latest/install.html


BlockSci Compilation Instructions
======================================

Here are the steps for compiling BlockSci on Ubuntu 16.04 (only tested on Ubuntu 16.04).

Note that BlockSci only actively supports python 3.

..  code-block:: bash

	sudo apt-get update
	sudo apt install build-essential cmake libssl-dev libboost-all-dev libsqlite3-dev autogen \
	autoconf libleveldb-dev libcurl4-openssl-dev libjsoncpp-dev libjsonrpccpp-dev libjsonrpccpp-tools \
	python3-dev python3-pip
	sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
	sudo apt-get update
	sudo apt install gcc-6 g++-6
	sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-6 60 --slave /usr/bin/g++ g++ /usr/bin/g++-6

	git clone https://github.com/bitcoin-core/secp256k1
	cd secp256k1
	./autogen.sh
	./configure --enable-module-recovery
	make
	sudo make install
	
	git clone https://github.com/cryptolu/BlockSci.git
	cd BlockSci
	git submodule update --init --recursive --checkout

	cd libs/bitcoin-api-cpp
	mkdir release
	cd release
	cmake --DCMAKE_BUILD_TYPE=Release ..
	make
	sudo make install

	cd ../../..
	mkdir release
	cd release
	cmake --DCMAKE_BUILD_TYPE=Release ..
	make
	sudo make install

	sudo -H pip3 install --upgrade pip
	sudo -H pip3 install --upgrade multiprocess psutil jupyter pycrypto matplotlib pandas dateparser
	

.. _paper: https://cryptolux.org/images/d/d9/Zcash.pdf
