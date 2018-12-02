import blocksci


def test_power_of_ten_change(chain, json_data):
    for i in range(6):
        txid = json_data["change-ten-{}-tx".format(i)]
        tx = chain.tx_with_hash(txid)
        result = blocksci.heuristics.change.power_of_ten_value(8 - i, tx)
        assert 1 == len(result), "Incorrect number of outputs identified with {} digits".format(i)

        idx = json_data["change-ten-{}-position".format(i)]
        assert idx == list(result)[0].index, "Incorrect index of change output"


def test_peeling_chain_change(chain, json_data):
    for i in range(3, 8):
        txid = json_data["peeling-chain-{}-tx".format(i)]
        tx = chain.tx_with_hash(txid)
        result = blocksci.heuristics.change.peeling_chain(tx)
        assert 1 == len(result)

        idx = json_data["peeling-chain-{}-position".format(i)]
        assert idx == list(result)[0].index


def test_optimal_change(chain, json_data):
    for i in range(2):
        txid = json_data["change-optimal-{}-tx".format(i)]
        tx = chain.tx_with_hash(txid)

        result = blocksci.heuristics.change.optimal_change(tx)
        assert 1 == len(result)

        idx = json_data["change-optimal-{}-position".format(i)]
        assert idx == list(result)[0].index


def test_address_type_change(chain, json_data):
    for i in range(3):
        txid = json_data["change-address-type-tx-{}".format(i)]
        tx = chain.tx_with_hash(txid)

        result = blocksci.heuristics.change.address_type(tx)
        assert 1 == len(result)

        idx = json_data["change-address-type-position-{}".format(i)]
        assert idx == list(result)[0].index


def test_locktime_change(chain, json_data):
    for i in range(3):
        txid = json_data["change-locktime-tx-{}".format(i)]
        tx = chain.tx_with_hash(txid)

        result = blocksci.heuristics.change.locktime(tx)
        assert 1 == len(result)

        idx = json_data["change-locktime-position-{}".format(i)]
        assert idx == list(result)[0].index


def test_address_reuse_change(chain, json_data):
    for i in range(3):
        txid = json_data["change-reuse-tx-{}".format(i)]
        tx = chain.tx_with_hash(txid)

        result = blocksci.heuristics.change.address_reuse(tx)
        assert 1 == len(result)

        idx = json_data["change-reuse-position-{}".format(i)]
        assert idx == list(result)[0].index


def test_address_client_behavior_change(chain, json_data):
    # TODO: write a correct test case
    # changing the generated pattern will break some of the regression tests
    pass


def test_no_change(chain, json_data):
    test_txs = ['change-reuse-tx-{}', 'change-locktime-tx-{}', 'change-address-type-tx-{}']
    for test in test_txs:
        for i in range(3):
            txid = json_data[test.format(i)]
            tx = chain.tx_with_hash(txid)

            result = blocksci.heuristics.change.none(tx)
            assert 0 == len(result)
