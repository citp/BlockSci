import blocksci
import pytest
from util import Coin, sorted_tx_list


def address_types(chain_name):
    if chain_name == "btc":
        return ["p2pkh", "p2sh", "p2wpkh", "p2wsh"]
    else:
        return ["p2pkh", "p2sh"]


def addresses(chain, json_data, chain_name):
    for addr_type in address_types(chain_name):
        for i in range(3):
            addr = chain.address_from_string(
                json_data["address-{}-spend-{}".format(addr_type, i)]
            )
            yield addr, addr_type

    raw_multisig_tx = chain.tx_with_hash(json_data["raw-multisig-tx"])
    yield raw_multisig_tx.outputs[0].address, "raw-multisig"

    p2sh_multisig_tx = chain.tx_with_hash(json_data["p2sh-multisig-tx"])
    yield p2sh_multisig_tx.outputs[0].address, "p2sh-multisig"


def address_received_test(addr, address_type, balance, ntxes):
    assert address_type == addr.type
    assert Coin(balance) == addr.balance()
    assert ntxes == len(addr.txes.to_list())


def test_p2pkh_address(chain, json_data):
    addr = chain.address_from_string(json_data["address-p2pkh-spend-0"])
    address_received_test(addr, blocksci.address_type.pubkeyhash, 1, 1)

    addr = chain.address_from_string(json_data["address-p2pkh-spend-1"])
    address_received_test(addr, blocksci.address_type.pubkeyhash, 0, 2)

    addr = chain.address_from_string(json_data["address-p2pkh-spend-2"])
    address_received_test(addr, blocksci.address_type.pubkeyhash, 0, 4)


def test_p2sh_address(chain, json_data):
    addr = chain.address_from_string(json_data["address-p2sh-spend-0"])
    address_received_test(addr, blocksci.address_type.scripthash, 1, 1)

    addr = chain.address_from_string(json_data["address-p2sh-spend-1"])
    address_received_test(addr, blocksci.address_type.scripthash, 0, 2)

    addr = chain.address_from_string(json_data["address-p2sh-spend-2"])
    address_received_test(addr, blocksci.address_type.scripthash, 0, 4)


@pytest.mark.btc
@pytest.mark.ltc
def test_p2wpkh_address(chain, json_data):
    addr = chain.address_from_string(json_data["address-p2wpkh-spend-0"])
    address_received_test(addr, blocksci.address_type.witness_pubkeyhash, 1, 1)

    addr = chain.address_from_string(json_data["address-p2wpkh-spend-1"])
    address_received_test(addr, blocksci.address_type.witness_pubkeyhash, 0, 2)

    addr = chain.address_from_string(json_data["address-p2wpkh-spend-2"])
    address_received_test(addr, blocksci.address_type.witness_pubkeyhash, 0, 4)


@pytest.mark.btc
@pytest.mark.ltc
def test_p2wsh_address(chain, json_data):
    addr = chain.address_from_string(json_data["address-p2wsh-spend-0"])
    address_received_test(addr, blocksci.address_type.witness_scripthash, 1, 1)

    addr = chain.address_from_string(json_data["address-p2wsh-spend-1"])
    address_received_test(addr, blocksci.address_type.witness_scripthash, 0, 2)

    addr = chain.address_from_string(json_data["address-p2wsh-spend-2"])
    address_received_test(addr, blocksci.address_type.witness_scripthash, 0, 4)


def test_address_regression(chain, json_data, regtest, chain_name):
    for addr, addr_type in addresses(chain, json_data, chain_name):
        if "multisig" not in addr_type:
            print(addr.address_string, file=regtest)
        print(addr.balance(), file=regtest)
        print(addr.equiv(), file=regtest)
        print(addr.has_been_spent, file=regtest)
        print(addr.inputs.to_list(), file=regtest)
        print(sorted_tx_list(addr.input_txes), file=regtest)
        print(addr.input_txes_count(), file=regtest)
        print(sorted_tx_list(addr.output_txes), file=regtest)
        print(addr.output_txes_count(), file=regtest)
        print(addr.outputs.to_list(), file=regtest)
        if addr_type == "p2pkh" or addr_type == "p2wpkh":
            print(addr.pubkey, file=regtest)
            print(addr.pubkeyhash, file=regtest)
        print(addr.raw_type, file=regtest)
        print(addr.revealed_tx, file=regtest)
        print(addr.type, file=regtest)
        print(addr.full_type, file=regtest)
        if addr.type == blocksci.address_type.multisig:
            print(addr.required, file=regtest)
            print(addr.total, file=regtest)
        if "p2sh" in addr_type or addr_type == "p2wsh":
            print(addr.wrapped_address, file=regtest)
