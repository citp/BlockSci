def map_block_tx_count(chain):
    offset = min(len(chain), 10000)
    assert chain[-offset:].map(lambda b: b.tx_count).size


def test_proxy_map_block(chain, benchmark):
    benchmark(map_block_tx_count, chain)


def map_tx_locktime(chain):
    assert chain[-100:].txes.map(lambda tx: tx.locktime).size


def test_proxy_map_tx_locktime(chain, benchmark):
    benchmark(map_tx_locktime, chain)


def where_tx_locktime(chain):
    assert chain[-100:].txes.where(lambda tx: tx.locktime > 0)


def test_proxy_where_tx_locktime(chain, benchmark):
    benchmark(where_tx_locktime, chain)


def where_output_is_spent(chain):
    assert chain[-100:].txes.outputs.where(lambda o: o.is_spent)


def test_proxy_where_output_is_spent(chain, benchmark):
    benchmark(where_output_is_spent, chain)


def size_blocks(chain):
    offset = min(len(chain), 10000)
    assert offset == chain[-offset:].size


def test_proxy_size_blocks(benchmark, chain):
    benchmark(size_blocks, chain)


def size_txes(chain):
    assert chain[-100:].txes.size > 0


def test_proxy_size_txes(benchmark, chain):
    benchmark(size_txes, chain)


def max_tx_fee(chain):
    assert chain[-100:].txes.max(lambda tx: tx.fee)


def test_proxy_max_tx_fee(chain, benchmark):
    benchmark(max_tx_fee, chain)


def max_output_value(chain):
    assert chain[-100:].txes.outputs.max(lambda o: o.value)


def test_proxy_max_output_value(chain, benchmark):
    benchmark(max_output_value, chain)


def min_tx_fee(chain):
    assert chain[-100:].txes.min(lambda tx: tx.fee)


def test_proxy_min_tx_fee(chain, benchmark):
    benchmark(min_tx_fee, chain)


def min_output_value(chain):
    assert chain[-100:].txes.outputs.min(lambda o: o.value)


def test_proxy_min_output_value(chain, benchmark):
    benchmark(min_output_value, chain)


def txes_any_output_spent(chain):
    assert chain[-100:].txes.where(lambda tx: tx.outputs.any(lambda o: o.is_spent))


def test_proxy_txes_any_output_spent(chain, benchmark):
    benchmark(txes_any_output_spent, chain)


def txes_all_outputs_spent(chain):
    assert chain[-100:].txes.where(lambda tx: tx.outputs.all(lambda o: o.is_spent))


def test_proxy_txes_all_outputs_spent(chain, benchmark):
    benchmark(txes_all_outputs_spent, chain)


def group_by_utxo_type_value(chain):
    chain.blocks.txes.outputs.where(lambda o: o.is_spent).group_by(lambda o: o.address_type, lambda o: o.value.sum)


def test_proxy_group_by_utxo_type_value(chain, benchmark):
    benchmark(group_by_utxo_type_value, chain)
