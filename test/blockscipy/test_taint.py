import blocksci
from util import FEE, Coin


def total_output_value(taint_result):
    return sum(x[0].value for x in taint_result)


def total_tainted_value(taint_result):
    return sum(x[1][0] for x in taint_result)


def total_untainted_value(taint_result):
    return sum(x[1][1] for x in taint_result)


def test_poison_single_output_no_fee(chain, json_data):
    out_1 = chain.tx_with_hash(json_data['taint-split-tx-1']).outputs[0]

    # taint one output until 1 block after
    result = blocksci.heuristics.poison_tainted_outputs([out_1], taint_fee=False,
                                                        max_block_height=out_1.tx.block_height + 1)
    assert 1 == len(result)
    assert Coin(3 - 3 * FEE) == total_output_value(result)
    assert Coin(3 - 3 * FEE) == total_tainted_value(result)
    assert 0 == total_untainted_value(result)

    # taint one output until the end
    result = blocksci.heuristics.poison_tainted_outputs([out_1], taint_fee=False)
    assert 1 == len(result)
    assert Coin(15 - 6 * FEE) == total_output_value(result)
    assert Coin(15 - 6 * FEE) == total_tainted_value(result)
    assert 0 == total_untainted_value(result)


def test_poison_two_outputs_no_fee(chain, json_data):
    out_1 = chain.tx_with_hash(json_data['taint-split-tx-1']).outputs[0]
    out_2 = chain.tx_with_hash(json_data['taint-fund-tx-2']).outputs[0]

    # taint two outputs until 1 block after
    result = blocksci.heuristics.poison_tainted_outputs([out_1, out_2], taint_fee=False,
                                                        max_block_height=out_1.tx.block_height + 1)
    assert 2 == len(result)
    assert Coin(15 - 5 * FEE) == total_output_value(result)
    assert Coin(15 - 5 * FEE) == total_tainted_value(result)
    assert 0 == total_untainted_value(result)

    # taint two outputs until the end
    result = blocksci.heuristics.poison_tainted_outputs([out_1, out_2], taint_fee=False)
    assert 1 == len(result)
    assert Coin(15 - 6 * FEE) == total_output_value(result)
    assert Coin(15 - 6 * FEE) == total_tainted_value(result)
    assert 0 == total_untainted_value(result)


def test_poison_single_output_with_fee(chain, json_data):
    out_1 = chain.tx_with_hash(json_data['taint-split-tx-1']).outputs[0]
    # taint one output for one block
    result = blocksci.heuristics.poison_tainted_outputs([out_1], taint_fee=True,
                                                        max_block_height=out_1.tx.block_height + 1)
    assert 2 == len(result)
    assert Coin(25 + 3 - FEE) == total_output_value(result)
    assert Coin(25 + 3 - FEE) == total_tainted_value(result)
    assert 0 == total_untainted_value(result)

    # taint one output for two blocks
    result = blocksci.heuristics.poison_tainted_outputs([out_1], taint_fee=True,
                                                        max_block_height=out_1.tx.block_height + 2)
    assert 3 == len(result)
    assert Coin(50 + 15 - 3 * FEE) == total_output_value(result)
    assert Coin(50 + 15 - 3 * FEE) == total_tainted_value(result)
    assert 0 == total_untainted_value(result)

    # taint one output until the end
    result = blocksci.heuristics.poison_tainted_outputs([out_1], taint_fee=True)
    assert 3 == len(result)
    assert Coin(50 + 15 - 3 * FEE) == total_output_value(result)
    assert Coin(50 + 15 - 3 * FEE) == total_tainted_value(result)
    assert 0 == total_untainted_value(result)


def test_poison_two_outputs_with_fee(chain, json_data):
    out_1 = chain.tx_with_hash(json_data['taint-split-tx-1']).outputs[0]
    out_2 = chain.tx_with_hash(json_data['taint-fund-tx-2']).outputs[0]

    # taint two outputs until 1 block after
    result = blocksci.heuristics.poison_tainted_outputs([out_1, out_2], taint_fee=True,
                                                        max_block_height=out_1.tx.block_height + 1)
    assert 3 == len(result)
    assert Coin(15 + 25 - 3 * FEE) == total_output_value(result)
    assert Coin(15 + 25 - 3 * FEE) == total_tainted_value(result)
    assert 0 == total_untainted_value(result)

    # taint two outputs until the end
    result = blocksci.heuristics.poison_tainted_outputs([out_1, out_2], taint_fee=True)
    assert 3 == len(result)
    assert Coin(15 + 50 - 3 * FEE) == total_output_value(result)
    assert Coin(15 + 50 - 3 * FEE) == total_tainted_value(result)
    assert 0 == total_untainted_value(result)


def test_poison_mapping(chain, json_data):
    out = chain.tx_with_hash(json_data['taint-mapping-fund-tx-2']).outputs[0]
    result = blocksci.heuristics.poison_tainted_outputs([out], max_block_height=out.tx.block_height + 1)
    assert 4 == len(result)
    assert Coin(35) == total_output_value(result)
    assert Coin(35) == total_tainted_value(result)


def test_haircut_single_output_no_fee(chain, json_data):
    out_1 = chain.tx_with_hash(json_data['taint-split-tx-1']).outputs[0]

    # taint one output until 1 block after
    result = blocksci.heuristics.haircut_tainted_outputs([out_1], taint_fee=False,
                                                         max_block_height=out_1.tx.block_height + 1)
    assert 1 == len(result)
    assert Coin(3 - 3 * FEE) == total_output_value(result)
    assert Coin(1 - FEE - ((1 - FEE) / (3 - 2 * FEE) * FEE)) - 1 == total_tainted_value(result)  # -1 for rounding error
    assert 0 != total_untainted_value(result)

    # taint one output until the end
    result = blocksci.heuristics.haircut_tainted_outputs([out_1], taint_fee=False)
    assert 1 == len(result)
    assert Coin(15 - 6 * FEE) == total_output_value(result)
    assert 99986000 - 1 == total_tainted_value(result)  # -1 for rounding error
    assert 0 != total_untainted_value(result)


def test_haircut_single_output_with_fee(chain, json_data):
    out_1 = chain.tx_with_hash(json_data['taint-split-tx-1']).outputs[0]
    # taint one output for one block
    result = blocksci.heuristics.haircut_tainted_outputs([out_1], taint_fee=True,
                                                         max_block_height=out_1.tx.block_height + 1)
    print(result)
    assert 2 == len(result)
    assert Coin(25 + 3 - FEE) == total_output_value(result)
    assert Coin(1 - FEE) == total_tainted_value(result)
    assert 0 != total_untainted_value(result)

    # taint one output for two blocks
    result = blocksci.heuristics.haircut_tainted_outputs([out_1], taint_fee=True,
                                                         max_block_height=out_1.tx.block_height + 2)
    assert 3 == len(result)
    assert Coin(50 + 15 - 3 * FEE) == total_output_value(result)
    assert Coin(1 - FEE) == total_tainted_value(result)
    assert 0 != total_untainted_value(result)

    # taint one output until the end
    result = blocksci.heuristics.haircut_tainted_outputs([out_1], taint_fee=True)
    assert 3 == len(result)
    assert Coin(50 + 15 - 3 * FEE) == total_output_value(result)
    assert Coin(1 - FEE) == total_tainted_value(result)
    assert 0 != total_untainted_value(result)


def test_haircut_two_outputs_with_fee(chain, json_data):
    out_1 = chain.tx_with_hash(json_data['taint-split-tx-1']).outputs[0]
    out_2 = chain.tx_with_hash(json_data['taint-fund-tx-2']).outputs[0]

    # taint two outputs until 1 block after
    result = blocksci.heuristics.haircut_tainted_outputs([out_1, out_2], taint_fee=True,
                                                         max_block_height=out_1.tx.block_height + 1)
    assert 3 == len(result)
    assert Coin(15 + 25 - 3 * FEE) == total_output_value(result)
    assert Coin(9 - FEE) == total_tainted_value(result)
    assert 0 != total_untainted_value(result)

    # taint two outputs until the end
    result = blocksci.heuristics.haircut_tainted_outputs([out_1, out_2], taint_fee=True)
    assert 3 == len(result)
    assert Coin(15 + 50 - 3 * FEE) == total_output_value(result)
    assert Coin(9 - FEE) == total_tainted_value(result)
    assert 0 != total_untainted_value(result)


def test_haircut_mapping(chain, json_data):
    out = chain.tx_with_hash(json_data['taint-mapping-fund-tx-2']).outputs[0]
    result = blocksci.heuristics.haircut_tainted_outputs([out], max_block_height=out.tx.block_height + 1)
    assert 4 == len(result)
    assert Coin(35) == total_output_value(result)
    assert Coin(4) == total_tainted_value(result)
