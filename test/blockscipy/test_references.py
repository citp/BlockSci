from test_addresses import address_types


def test_block_references(chain):
    for i in range(1, len(chain)):
        assert chain[i - 1] == chain[i].prev_block


def test_tx_references(chain, json_data, chain_name):
    for addr_type in address_types(chain_name):
        for i in range(3):
            tx = chain.tx_with_hash(json_data["address-{}-spend-{}-tx".format(addr_type, i)])
            height = json_data["address-{}-spend-{}-height".format(addr_type, i)]

            assert chain[height] == tx.block


def test_output_references(chain, json_data, chain_name):
    for addr_type in address_types(chain_name):
        for i in range(3):
            tx = chain.tx_with_hash(json_data["address-{}-spend-{}-tx".format(addr_type, i)])
            height = json_data["address-{}-spend-{}-height".format(addr_type, i)]

            for j in range(tx.output_count):
                assert tx == tx.outputs[j].tx
                assert chain[height] == tx.outputs[j].tx.block


def test_spending_tx_references(chain, json_data):
    for i in range(8):
        tx = chain.tx_with_hash(json_data["peeling-chain-{}-tx".format(i)])
        next_tx = chain.tx_with_hash(json_data["peeling-chain-{}-tx".format(i + 1)])

        assert next_tx == tx.outputs[json_data["peeling-chain-{}-position".format(i)]].spending_tx
        assert tx == next_tx.inputs[0].spent_tx


def test_spent_output_references(chain, json_data):
    for i in range(8):
        tx = chain.tx_with_hash(json_data["peeling-chain-{}-tx".format(i + 1)])
        prev_tx = chain.tx_with_hash(json_data["peeling-chain-{}-tx".format(i)])
        output_index = json_data["peeling-chain-{}-position".format(i)]

        assert tx.inputs[0].spent_output
        assert output_index == tx.inputs[0].spent_output.index
        assert prev_tx == tx.inputs[0].spent_output.tx


def test_spending_input_references(chain, json_data):
    tx = chain.tx_with_hash(json_data["fan-8-tx"])
    redeeming_tx = chain.tx_with_hash(json_data["fan-8-tx-collect"])
    for idx, out in enumerate(tx.outputs):
        assert redeeming_tx.inputs[idx] == out.spending_input


def test_address_references(chain, json_data, chain_name):
    for addr_type in address_types(chain_name):
        for i in range(2):
            addr = chain.address_from_string(json_data["address-{}-spend-{}".format(addr_type, i)])
            tx = chain.tx_with_hash(json_data["address-{}-spend-{}-tx".format(addr_type, i)])
            height = json_data["address-{}-spend-{}-height".format(addr_type, i)]

            for j in range(i):
                assert tx == addr.outs.to_list()[j].tx
                assert chain[height] == addr.outs.to_list()[j].tx.block
