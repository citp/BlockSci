import pytest
import blocksci


def compute_fee_naive(chain):
    return sum(
        [sum(txin.value for txin in tx.inputs) - sum(txout.value for txout in tx.outputs) for block in chain for tx in
         block if not tx.is_coinbase])


def compute_fee_builtin(chain):
    return sum([tx.fee for block in chain for tx in block if not tx.is_coinbase])


def compute_fee_proxy(chain):
    return chain.blocks.txes.where(lambda tx: ~tx.is_coinbase).fee.sum()


@pytest.mark.btc
def test_fee_naive(chain, benchmark):
    benchmark(compute_fee_naive, chain)


@pytest.mark.btc
def test_fee_builtin(chain, benchmark):
    benchmark(compute_fee_builtin, chain)


def test_fee_proxy(chain, benchmark):
    benchmark(compute_fee_proxy, chain)
