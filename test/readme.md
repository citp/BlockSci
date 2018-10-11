# Testing

The `blockscipy` directory contains functional and regression tests.
The tests are based on a synthetic blockchain, which is deterministically generated using Bitcoin's regtest mode.

## Setup

You'll need to install `pytest` and the `pytest-regtest` plugin:

- `pip3 install pytest`
- `pip3 install pytest-regtest`

## Running the tests

- `cd blockscipy` (this is important, otherwise it won't find the previous results of regression test)
- `pytest` to run all tests, or 
- `pytest test_xyz.py` to just run test `xyz.py`
