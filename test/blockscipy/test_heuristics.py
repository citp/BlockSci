# change heuristics are tested in test_change.py

import blocksci


def test_simple_coinjoin(chain, json_data):
    tx = chain.tx_with_hash(json_data["simple-coinjoin-tx"])
    assert blocksci.heuristics.is_coinjoin(tx)


def test_no_coinjoin(chain, json_data):
    for key in [
        "fan-8-tx",
        "peeling-chain-4-tx",
        "tx-chain-10-tx-1",
        "funding-tx-2-in-2-out",
    ]:
        tx = chain.tx_with_hash(json_data[key])
        assert not blocksci.heuristics.is_coinjoin(tx)


def test_is_peeling_chain(chain, json_data):
    for i in range(0, 9):
        txid = json_data["peeling-chain-{}-tx".format(i)]
        tx = chain.tx_with_hash(txid)
        assert blocksci.heuristics.is_peeling_chain(tx)


def test_no_peeling_chain(chain, json_data):
    for key in [
        "fan-8-tx",
        "simple-coinjoin-tx",
        "tx-chain-10-tx-1",
    ]:
        tx = chain.tx_with_hash(json_data[key])
        assert not blocksci.heuristics.is_peeling_chain(tx)
