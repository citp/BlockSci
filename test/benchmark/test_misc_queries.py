import pytest
import blocksci


def count_blocks(chain):
    return len(chain)


def positive_locktime(chain):
    return chain.blocks.txes.where(lambda tx: tx.locktime > 0).size


def max_output_value(chain):
    return chain.blocks.outputs.max(lambda o: o.value).value


def max_fee(chain):
    return chain.blocks.txes.max(lambda tx: tx.fee).fee


def locktime_change(chain):
    heuristic = (blocksci.heuristics.change.spent & blocksci.heuristics.change.locktime)
    return chain.blocks.txes.where(lambda tx: heuristic(tx).size == 1).size


def zero_conf(chain):
    return chain.blocks.txes.outputs.where(
        lambda o: o.spending_tx.map(lambda t: t.block_height).or_value(999999) == o.tx.block_height).size


def satoshi_dice_outputs(chain):
    address = chain.address_from_string("1dice97ECuByXAvqXpaYzSaQuPVvrtmz6")
    return address.outs.value.sum()


def test_query_count_blocks(chain, benchmark):
    benchmark(count_blocks, chain)


def test_query_positive_locktime(chain, benchmark):
    benchmark(positive_locktime, chain)


def test_query_max_output_value(chain, benchmark):
    benchmark(max_output_value, chain)


def test_query_max_fee(chain, benchmark):
    benchmark(max_fee, chain)


def test_query_locktime_change(chain, benchmark):
    benchmark(locktime_change, chain)


def test_query_zero_conf(chain, benchmark):
    benchmark(zero_conf, chain)


@pytest.mark.local
def test_query_satoshi_dice_outputs(chain, benchmark):
    benchmark(satoshi_dice_outputs, chain)
