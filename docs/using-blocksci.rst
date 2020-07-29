Using BlockSci
=========================

After installing BlockSci and parsing a compatible blockchain, the analysis library is ready for use.


Python
-----------

To use BlockSci in Python, you only need to import the BlockSci library. Then, you can instantiate a :py:class:`blocksci.Blockchain` object by pointing it to the config file.

.. code-block:: python

    import blocksci
    chain = blocksci.Blockchain("path/to/your/config.json")


.. For more examples, refer to the *introduction notebook* on GitHub.


If you would like to use BlockSci through a web interface, we recommend the use of `Jupyter Lab`_.

.. _Jupyter Lab: https://jupyter.readthedocs.io/en/latest/install.html


C++
---------

In order to use the C++ library, you must compile your code against the BlockSci dynamic library and add its headers to your include path. The Blockchain can then be constructed given the path to the config file.

.. code-block:: c++

    #include <blocksci/blocksci.hpp>

    int main(int argc, const char * argv[]) {
            blocksci::Blockchain chain{"path/to/your/config.json"};
    }

An easy way to get started is to modify the file ``example/example.cpp``. Then, compile and run it as follows:

.. code-block:: bash

    cd release
    make blocksci_example
    ./example/blocksci_example <path to config>

