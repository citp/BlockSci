def test_map_txes(chain):
    tx0 = set()
    for block in chain:
        for tx in block:
            tx0.add(tx.index)

    tx1 = set(chain.blocks.txes.index)
    tx2 = set(chain.blocks.map(lambda b: b.txes).index)
    tx3 = set(chain.blocks.map(lambda b: b.txes.index))
    tx4 = set(chain.blocks.map(lambda b: b.txes.map(lambda tx: tx.index)))
    tx5 = set(chain.blocks.map(lambda b: b.txes).map(lambda tx: tx.index))

    assert tx1 == tx0
    assert tx2 == tx0
    assert tx3 == tx0
    assert tx4 == tx0
    assert tx5 == tx0


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


def test_max(chain):
    max_out_value = -1
    for block in chain:
        for tx in block:
            for out in tx.outputs:
                max_out_value = max(out.value, max_out_value)

    assert max_out_value == chain.blocks.txes.outputs.max(lambda o: o.value).value


def test_min(chain):
    min_out_value = 9999999999
    for block in chain:
        for tx in block:
            for out in tx.outputs:
                min_out_value = min(out.value, min_out_value)

    assert min_out_value == chain.blocks.txes.outputs.min(lambda o: o.value).value


def test_all(chain, json_data):
    tx = chain.tx_with_hash(json_data["change-address-type-tx-0"])
    value = tx.outputs[0].value
    assert tx.outputs.any(lambda o: o.value == value)
    assert tx.outputs.all(lambda o: o.value == value)


def test_any(chain, json_data):
    tx = chain.tx_with_hash(json_data["change-negative-testcase-tx"])
    value = int(json_data["change-negative-testcase-value-0"])
    assert tx.outputs.any(lambda o: o.value == value)
    assert not tx.outputs.all(lambda o: o.value == value)

    value += 1
    assert not tx.outputs.any(lambda o: o.value == value)


def test_where_block_height(chain):
    assert set(range(120)) == set(chain.blocks.where(lambda b: b.height < 120).height)
    assert set(range(120, 140)) == set(
        chain.blocks.where(lambda b: (b.height >= 120) & (b.height < 140)).height
    )
    assert set(range(120, 140)) == set(
        chain.blocks.where(lambda b: b.height >= 120)
        .where(lambda b: b.height < 140)
        .height
    )


def test_where_tx_locktime(chain):
    txs = set()
    for block in chain:
        for tx in block:
            if tx.locktime > 0:
                txs.add(tx.index)
    assert set(chain.blocks.txes.where(lambda tx: tx.locktime > 0).index) == txs


def test_where_address(chain):
    a1 = chain.blocks[1].txes[0].outputs[0].address

    proxy_txes = chain.blocks.txes.where(
        lambda tx: tx.inputs.address.any(lambda a: a == a1)
        | tx.outputs.address.any(lambda b: b == a1)
    )

    assert set(a1.txes) == set(proxy_txes)


def check_iterator_range(rng, regtest):
    s = rng.size
    assert s == len(rng.to_list())
    print(s, file=regtest)


def test_size(chain, regtest):
    check_iterator_range(chain.blocks, regtest)
    check_iterator_range(chain.blocks.txes, regtest)
    check_iterator_range(chain.blocks.txes.inputs, regtest)
    check_iterator_range(chain.blocks.txes.outputs, regtest)

    check_iterator_range(chain[121].txes, regtest)
    check_iterator_range(chain[121].txes.inputs, regtest)
    check_iterator_range(chain[121].txes.outputs, regtest)
