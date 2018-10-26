import blocksci
from util import FEE, Coin


def test_poison_no_fee(chain, json_data):
    # taint one output until the end
    out_1 = chain.tx_with_hash(json_data['taint-split-tx-1']).outputs[0]
    result = blocksci.heuristics.poison_tainted_outputs([out_1], taint_fee=False)
    assert 1 == len(result)
    assert Coin(15 - 6 * FEE) == sum(o[0].value for o in result)

    # taint one output until 1 block after
    result = blocksci.heuristics.poison_tainted_outputs([out_1], taint_fee=False,
                                                        max_block_height=out_1.tx.block_height + 1)
    assert 1 == len(result)
    assert Coin(3 - 3 * FEE) == result[0][0].value
    assert Coin(3 - 3 * FEE) == result[0][1][0]

    # taint two outputs until the end
    out_2 = chain.tx_with_hash(json_data['taint-fund-tx-2']).outputs[0]
    result = blocksci.heuristics.poison_tainted_outputs([out_1, out_2], taint_fee=False)
    assert 1 == len(result)
    assert Coin(15 - 6 * FEE) == sum(o[0].value for o in result)

    # taint two outputs until 1 block after
    result = blocksci.heuristics.poison_tainted_outputs([out_1, out_2], taint_fee=False,
                                                        max_block_height=out_1.tx.block_height + 1)
    assert 2 == len(result)
    assert Coin(15 - 5 * FEE) == sum(o[0].value for o in result)


def test_poison_with_fee(chain, json_data):
    # taint one output for one block
    out_1 = chain.tx_with_hash(json_data['taint-split-tx-1']).outputs[0]
    result = blocksci.heuristics.poison_tainted_outputs([out_1], taint_fee=True, max_block_height=out_1.tx.block_height+1)
    assert Coin(25 + 3 - FEE) == sum(o[0].value for o in result)
    assert Coin(25 + 3 - FEE) == sum(o[1][0] for o in result)
    assert 2 == len(result)

    # taint one output for two blocks
    result = blocksci.heuristics.poison_tainted_outputs([out_1], taint_fee=True,
                                                        max_block_height=out_1.tx.block_height + 2)
    assert Coin(50 + 15 - 3 * FEE) == sum(o[0].value for o in result)
    assert Coin(50 + 15 - 3 * FEE) == sum(o[1][0] for o in result)
    assert 3 == len(result)

    # taint one output until the end
    result = blocksci.heuristics.poison_tainted_outputs([out_1], taint_fee=True)
    assert Coin(50 + 15 - 3 * FEE) == sum(o[0].value for o in result)
    assert Coin(50 + 15 - 3 * FEE) == sum(o[1][0] for o in result)
    assert 3 == len(result)

    # taint two outputs until 1 block after
    out_2 = chain.tx_with_hash(json_data['taint-fund-tx-2']).outputs[0]
    result = blocksci.heuristics.poison_tainted_outputs([out_1, out_2], taint_fee=True,
                                                        max_block_height=out_1.tx.block_height + 1)
    assert 3 == len(result)
    assert Coin(15 + 25 - 3 * FEE) == sum(o[0].value for o in result)

    # taint two outputs until the end
    result = blocksci.heuristics.poison_tainted_outputs([out_1, out_2], taint_fee=True)
    assert 3 == len(result)
    assert Coin(15 + 50 - 3 * FEE) == sum(o[0].value for o in result)

