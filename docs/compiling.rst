Compiling BlockSci
=======================

BlockSci supports GCC 7.2 or above and Clang 5 or above.

Note that BlockSci only actively supports Python 3.

If you have previously had BlockSci v0.4.5 or below installed you must first remove the existing installations

..  code-block:: bash

	rm -rf /usr/local/include/blocksci
	rm /usr/local/lib/libblocksci*
	rm /usr/local/bin/blocksci*
	rm /usr/local/bin/mempool_recorder

Ubuntu 18.04
---------------------

..  code-block:: bash

	sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
	sudo apt-get update
	sudo apt install cmake libtool autoconf libboost-filesystem-dev libboost-iostreams-dev \
	libboost-serialization-dev libboost-thread-dev libboost-test-dev  libssl-dev libjsoncpp-dev \
	libcurl4-openssl-dev libjsoncpp-dev libjsonrpccpp-dev libsnappy-dev zlib1g-dev libbz2-dev \
	liblz4-dev libzstd-dev libjemalloc-dev libsparsehash-dev python3-dev python3-pip

	git clone https://github.com/citp/BlockSci.git
	cd BlockSci
	mkdir release
	cd release
	CC=gcc-7 CXX=g++-7 cmake -DCMAKE_BUILD_TYPE=Release ..
	make
	sudo make install

	cd ..
	CC=gcc-7 CXX=g++-7 sudo -H pip3 install -e blockscipy


You will also need to increase_ the open files limit on your system.

.. _increase: https://ro-che.info/articles/2017-03-26-increase-open-files-limit


Mac OS 10.13
--------------

Compiling BlockSci with Xcode 11.5 currently requires a manual patch_.

.. _patch: https://github.com/citp/BlockSci/issues/412

..  code-block:: bash

    brew install cmake jsoncpp libjson-rpc-cpp boost openssl jemalloc zstd \
    automake libtool google-sparsehash lz4 python3
    sudo xcode-select --reset

    git clone https://github.com/citp/BlockSci.git
    cd BlockSci
    mkdir release
    cd release
    cmake -DCMAKE_BUILD_TYPE=Release -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl ..
    make
    sudo make install

    cd ..
	pip3 install -e blockscipy
