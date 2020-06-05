import pytest


def test_p2pkh_input_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2pkh-spend-1-tx"])
    input_regression(tx, regtest)


def test_p2sh_input_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2sh-spend-2-tx"])
    input_regression(tx, regtest)


@pytest.mark.btc
@pytest.mark.ltc
def test_p2wpkh_input_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2wpkh-spend-0-tx"])
    input_regression(tx, regtest)


@pytest.mark.btc
@pytest.mark.ltc
def test_p2wsh_input_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2wsh-spend-1-tx"])
    input_regression(tx, regtest)


def input_regression(tx, regtest):
    for inpt in tx.inputs:
        print(inpt.address, file=regtest)
        print(inpt.address_type, file=regtest)
        print(inpt.age, file=regtest)
        print(inpt.block, file=regtest)
        print(inpt.index, file=regtest)
        print(inpt.sequence_num, file=regtest)
        print(inpt.spent_output, file=regtest)
        print(inpt.spent_tx, file=regtest)
        print(inpt.spent_tx_index, file=regtest)
        print(inpt.tx, file=regtest)
        print(inpt.tx_index, file=regtest)
        print(inpt.value, file=regtest)
