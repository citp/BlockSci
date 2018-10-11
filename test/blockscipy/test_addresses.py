import blocksci
from util import Coin

FEE = 0.0001
ADDR_TYPES = ['p2pkh', 'p2sh', 'p2wpkh', 'p2wsh']


def addresses(chain, json_data):
    for addr_type in ADDR_TYPES:
        for i in range(3):
            addr = chain.address_from_string(json_data["address-{}-spend-{}".format(addr_type, i)])
            yield addr, addr_type


def address_received_test(addr, address_type, balance, ntxes):
    assert address_type == addr.type
    assert Coin(balance) == addr.balance()
    assert ntxes == len(addr.txes())


def test_p2pkh_address(chain, json_data):
    addr = chain.address_from_string(json_data['address-p2pkh-spend-0'])
    address_received_test(addr, blocksci.address_type.pubkeyhash, 1, 1)

    addr = chain.address_from_string(json_data['address-p2pkh-spend-1'])
    address_received_test(addr, blocksci.address_type.pubkeyhash, 0, 2)

    addr = chain.address_from_string(json_data['address-p2pkh-spend-2'])
    address_received_test(addr, blocksci.address_type.pubkeyhash, 0, 4)


def test_p2sh_address(chain, json_data):
    addr = chain.address_from_string(json_data['address-p2sh-spend-0'])
    address_received_test(addr, blocksci.address_type.scripthash, 1, 1)

    addr = chain.address_from_string(json_data['address-p2sh-spend-1'])
    address_received_test(addr, blocksci.address_type.scripthash, 0, 2)

    addr = chain.address_from_string(json_data['address-p2sh-spend-2'])
    address_received_test(addr, blocksci.address_type.scripthash, 0, 4)


def test_p2wpkh_address(chain, json_data):
    addr = chain.address_from_string(json_data['address-p2wpkh-spend-0'])
    address_received_test(addr, blocksci.address_type.witness_pubkeyhash, 1, 1)

    addr = chain.address_from_string(json_data['address-p2wpkh-spend-1'])
    address_received_test(addr, blocksci.address_type.witness_pubkeyhash, 0, 2)

    addr = chain.address_from_string(json_data['address-p2wpkh-spend-2'])
    address_received_test(addr, blocksci.address_type.witness_pubkeyhash, 0, 4)


def test_p2wsh_address(chain, json_data):
    addr = chain.address_from_string(json_data['address-p2wsh-spend-0'])
    address_received_test(addr, blocksci.address_type.witness_scripthash, 1, 1)

    addr = chain.address_from_string(json_data['address-p2wsh-spend-1'])
    address_received_test(addr, blocksci.address_type.witness_scripthash, 0, 2)

    addr = chain.address_from_string(json_data['address-p2wsh-spend-2'])
    address_received_test(addr, blocksci.address_type.witness_scripthash, 0, 4)


def test_address_regression(chain, json_data, regtest):
    for addr, addr_type in addresses(chain, json_data):
        print(addr.address_string, file=regtest)
        print(addr.balance(), file=regtest)
        print(addr.equiv(), file=regtest)
        print(addr.has_been_spent, file=regtest)
        print(addr.in_txes(), file=regtest)
        print(addr.in_txes_count(), file=regtest)
        print(addr.out_txes(), file=regtest)
        print(addr.out_txes_count(), file=regtest)
        if addr_type[-2:] != "sh":
            print(addr.pubkey, file=regtest)
            print(addr.pubkeyhash, file=regtest)
        print(addr.raw_type, file=regtest)
        print(addr.revealed_tx, file=regtest)
        print(addr.type, file=regtest)
