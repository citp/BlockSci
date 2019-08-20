def test_block_range(chain):
    blocks = [chain[idx] for idx in range(len(chain))]
    assert set(chain.blocks) == set(blocks)
    assert list(chain.blocks.fee) == [sum(tx.fee for tx in block) for block in blocks]
    assert list(chain.blocks.output_count) == [
        sum(tx.output_count for tx in block) for block in blocks
    ]


def test_tx_range(chain):
    txes = [tx for idx in range(len(chain)) for tx in chain[idx]]
    assert set(chain.blocks.txes) == set(txes)
    assert list(chain.blocks.txes.fee) == [tx.fee for tx in txes]
    assert list(chain.blocks.txes.output_count) == [tx.output_count for tx in txes]
    assert list(chain.blocks.txes.virtual_size) == [tx.virtual_size for tx in txes]
