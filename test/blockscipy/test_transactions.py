from util import tz_offset


def test_p2pkh_transaction_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2pkh-spend-0-tx"])
    transaction_regression(tx, regtest)


def test_p2sh_transaction_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2sh-spend-1-tx"])
    transaction_regression(tx, regtest)


def test_p2wpkh_transaction_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2wpkh-spend-2-tx"])
    transaction_regression(tx, regtest)


def test_p2wsh_transaction_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2wsh-spend-1-tx"])
    transaction_regression(tx, regtest)


def transaction_regression(tx, regtest):
    print(tx.base_size, file=regtest)
    print(tx.block_height, file=regtest)
    print(tx.block_time - tz_offset(tx.block_time), file=regtest)
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
