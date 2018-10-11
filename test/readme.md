# Testing

The `test/blockscipy` directory contains functional and regression tests.
These tests are based on a synthetic blockchain, which is deterministically generated using Bitcoin's regtest mode.

## Setup

You'll need to install `pytest` and the `pytest-regtest` plugin:

- `pip3 install pytest`
- `pip3 install pytest-regtest`

## Running the tests

- `cd blockscipy` (this is important, otherwise pytest won't find the previous results of the regression tests)
- `pytest` to run all tests, or 
- `pytest test_xyz.py` to run a single test
- `pytest test_xyz.py --regtest-reset` to store the current output of the regression tests
