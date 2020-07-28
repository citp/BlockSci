import pytest


def test_p2pkh_output_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2pkh-spend-2-tx"])
    output_regression(tx, regtest)


def test_p2sh_output_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2sh-spend-0-tx"])
    output_regression(tx, regtest)


@pytest.mark.btc
@pytest.mark.ltc
def test_p2wpkh_output_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2wpkh-spend-1-tx"])
    output_regression(tx, regtest)


@pytest.mark.btc
@pytest.mark.ltc
def test_p2wsh_output_regression(chain, json_data, regtest):
    tx = chain.tx_with_hash(json_data["address-p2wsh-spend-1-tx"])
    output_regression(tx, regtest)


def output_regression(tx, regtest):
    for out in tx.outputs:
        print(out.address, file=regtest)
        print(out.address_type, file=regtest)
        print(out.block, file=regtest)
        print(out.index, file=regtest)
        print(out.is_spent, file=regtest)
        print(out.spending_input, file=regtest)
        print(out.spending_tx, file=regtest)
        print(out.spending_tx_index, file=regtest)
        print(out.tx, file=regtest)
        print(out.tx_index, file=regtest)
        print(out.value, file=regtest)

def test_output_input_mapping(chain):
    for block in chain:
        for tx in block:
            for output in tx.outputs:
                if output.is_spent:
                    assert output == output.spending_input.spent_output
                    assert tx == output.spending_input.spent_tx
