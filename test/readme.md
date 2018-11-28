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

## Chain support

We currently test BlockSci with [synthetic blockchains](https://github.com/citp/testchain-generator) that were generated with Bitcoin Core for Bitcoin (BTC) and Bitcoin ABC for Bitcoin Cash (BCH).

### Running tests for a specific chain

To only run tests for Bitcoin, run `pytest --btc`. To run tests for Bitcoin Cash, run `pytest --bch`.

### Writing tests for specific chains

Use `@pytest.mark.btc` or `@pytest.mark.bch` if tests should only be run for a specific chain. For example, you might want to skip all tests involving Segwit addresses for Bitcoin Cash.
