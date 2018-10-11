def test_p2pkh_transaction_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2pkh-spend-0-tx"])
    transaction_regression(tx, regtest)


def test_p2pkh_input_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2pkh-spend-1-tx"])
    input_regression(tx, regtest)


def test_p2pkh_output_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2pkh-spend-2-tx"])
    output_regression(tx, regtest)


def test_p2sh_transaction_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2sh-spend-1-tx"])
    transaction_regression(tx, regtest)


def test_p2sh_input_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2sh-spend-2-tx"])
    input_regression(tx, regtest)


def test_p2sh_output_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2sh-spend-0-tx"])
    output_regression(tx, regtest)


def test_p2wpkh_transaction_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2wpkh-spend-2-tx"])
    transaction_regression(tx, regtest)


def test_p2wpkh_input_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2wpkh-spend-0-tx"])
    input_regression(tx, regtest)


def test_p2wpkh_output_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2wpkh-spend-1-tx"])
    output_regression(tx, regtest)


def test_p2wsh_transaction_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2wsh-spend-1-tx"])
    transaction_regression(tx, regtest)


def test_p2wsh_input_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2wsh-spend-1-tx"])
    input_regression(tx, regtest)


def test_p2wsh_output_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2wsh-spend-1-tx"])
    output_regression(tx, regtest)


def transaction_regression(tx, regtest):
    print(tx.base_size, file=regtest)
    print(tx.block_height, file=regtest)
    print(tx.block_time, file=regtest)
    print(tx.fee, file=regtest)
    print(tx.fee_per_byte(), file=regtest)
    print(tx.hash, file=regtest)
    print(tx.index, file=regtest)
    print(tx.is_coinbase, file=regtest)
    print(tx.locktime, file=regtest)
    print(tx.observed_in_mempool, file=regtest)
    print(tx.op_return, file=regtest)
    print(tx.output_count, file=regtest)
    print(tx.output_value, file=regtest)
    print(tx.size_bytes, file=regtest)
    print(tx.time_seen, file=regtest)
    print(tx.total_size, file=regtest)
    print(tx.virtual_size, file=regtest)
    print(tx.weight, file=regtest)


def input_regression(tx, regtest):
    for inpt in tx.inputs:
        print(inpt.address, file=regtest)
        print(inpt.address_type, file=regtest)
        print(inpt.age, file=regtest)
        print(inpt.block, file=regtest)
        print(inpt.index, file=regtest)
        print(inpt.sequence_num, file=regtest)
        print(inpt.spent_tx, file=regtest)
        print(inpt.spent_tx_index, file=regtest)
        print(inpt.tx, file=regtest)
        print(inpt.tx_index, file=regtest)
        print(inpt.value, file=regtest)


def output_regression(tx, regtest):
    for out in tx.outputs:
        print(out.address, file=regtest)
        print(out.address_type, file=regtest)
        print(out.block, file=regtest)
        print(out.index, file=regtest)
        print(out.is_spent, file=regtest)
        print(out.spending_tx, file=regtest)
        print(out.spending_tx_index, file=regtest)
        print(out.tx, file=regtest)
        print(out.tx_index, file=regtest)
        print(out.value, file=regtest)
