BlockSci
~~~~~~~~~~~~~~~~~~

BlockSci Documentation
=====================
Documentation_ is available for the python interface library

.. _Documentation: https://citp.github.io/BlockSci/

Additionally, a demonstration Notebook_ is available in the Notebooks folder

.. _Notebook: https://citp.github.io/BlockSci/demo.html


BlockSci Setup
=====================
In order to use BlockSci, you must be running a full node since BlockSci requires the full serialized blockchain data structure which full nodes produce. The blockchain_parser binary which is built from this repo performs the transformation of this blockchain data into our customized BlockSci database format.

The parser has two modes for creating data files. 

Disk Mode
-----------
Disk mode is optimized for parsing Bitcoin's data files. It reads blockchain data directly from disk in a rapid manner. However this means that it does not work on many other blockchains which have different serialization formats than Bitcoin.

..  code-block:: bash

	blocksci_parser --bitcoin-directory .bitcoin --output-directory bitcoin-data

RPC
------
RPC mode uses the RPC interface of a cryptocurrency to extract data regarding the blockchain. It works with a variety of cryptocurrencies which have the same general model as Bitcoin, but with minor changes to the serialization format which break the parser in disk mode. One example of this is Namecoin.

..  code-block:: bash

	blocksci_parser --username [user] --password [pass] --address [ip] --port [port] --output-directory bitcoin-data

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

To use the python library, you only need to import the blocksci library. By default the library is installed into BlockSci/Notebooks. To use the library simply open the python interpretter in that folder and run:

.. code-block:: python

	import blocksci
	chain = blocksci.Blockchain("file_path_to_output-directory")

BlockSci Installation Instructions
======================================

Below I list setup expressions for compiling blocksci on Ubuntu 16.04.

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

	cd libs/bitcoin-cpp-api
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
