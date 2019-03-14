def test_map_txes(chain):
    tx0 = set()
    for block in chain:
        for tx in block:
            tx0.add(tx.index)

    tx1 = set(chain.blocks.txes.index)
    tx2 = set(chain.blocks.map(lambda b: b.txes).index)
    tx3 = set(chain.blocks.map(lambda b: b.txes.index))

    assert tx1 == tx0
    assert tx2 == tx0
    assert tx3 == tx0


def test_map_outputs(chain):
    o0 = set()
    for block in chain:
        for tx in block:
            for o in tx.outputs:
                o0.add(o)

    o1 = set(chain.blocks.txes.outputs.to_list())
    o2 = set(chain.blocks.map(lambda b: b.txes.outputs).to_list())
    o3 = set(chain.blocks.map(lambda b: b.txes).outputs.to_list())

    assert o1 == o0
    assert o2 == o0
    assert o3 == o0


def test_where(chain):
    assert set(range(120)) == set(chain.blocks.where(lambda b: b.height < 120).height)



