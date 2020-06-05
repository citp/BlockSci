import pytest
import blocksci


def chain_length(chain):
    return len(chain)


def test_chain_length(chain, benchmark):
    benchmark(chain_length, chain)


def chain_slice(chain):
    chain_length = len(chain)
    mid = chain_length // 2
    interval = chain_length // 20
    start_ranges = [x for x in range(0, mid - 1, interval)]
    end_ranges = [x for x in range(mid, chain_length, interval)]

    lengths = []

    for i in range(len(start_ranges)):
        rng = chain[start_ranges[i]:end_ranges[i]]
        lengths.append(len(rng))


def test_chain_slice(chain, benchmark):
    benchmark(chain_slice, chain)


def iterate_blocks(chain):
    for height in range(0, len(chain)):
        assert chain[height].height == height


def test_chain_access_block(chain, benchmark):
    benchmark(iterate_blocks, chain)


def iterate_transactions(chain):
    offset = min(len(chain), 100)
    for block in chain[-offset:]:
        for idx in range(0, block.tx_count):
            assert block.txes[idx].locktime >= 0


def test_chain_access_txes(chain, benchmark):
    benchmark(iterate_transactions, chain)


def iterate_outputs_legacy(chain):
    offset = min(len(chain), 100)
    for block in chain[-offset:]:
        for tx in block:
            for output in tx.outputs:
                assert output.value >= 0


def test_chain_iterate_outputs_legacy(chain, benchmark):
    benchmark(iterate_outputs_legacy, chain)


def iterate_outputs_proxy(chain):
    for output in chain[-10:].txes.outputs:
        assert output.value >= 0


def test_chain_iterate_outputs_proxy(chain, benchmark):
    benchmark(iterate_outputs_proxy, chain)


def size_txes_outputs(chain):
    assert chain.blocks.txes.outputs.size


def test_chain_size_txes_outputs(chain, benchmark):
    benchmark(size_txes_outputs, chain)


def size_txes_outputs(chain):
    assert chain.blocks.outputs.size


def test_chain_size_blocks_outputs(chain, benchmark):
    benchmark(size_txes_outputs, chain)
