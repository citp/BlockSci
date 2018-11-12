# change heuristics are tested in test_change.py

import blocksci


def test_simple_coinjoin(chain, json_data):
    tx = chain.tx_with_hash(json_data['simple-coinjoin-tx'])
    assert blocksci.heuristics.is_coinjoin(tx)


def test_no_coinjoin(chain, json_data):
    for key in ['fan-8-tx', 'peeling-chain-4-tx', 'tx-chain-10-tx-1', 'funding-tx-2-in-2-out']:
        tx = chain.tx_with_hash(json_data[key])
        assert not blocksci.heuristics.is_coinjoin(tx)
