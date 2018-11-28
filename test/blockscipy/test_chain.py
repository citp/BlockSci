import pytest
import blocksci
from util import correct_timestamp


def test_chain_sanity(chain):
    assert 110 < len(chain)


def test_chain(chain, regtest):
    assert 0 < chain.address_count(blocksci.address_type.pubkey)
    assert 0 < chain.address_count(blocksci.address_type.pubkeyhash)
    assert 0 < chain.address_count(blocksci.address_type.scripthash)
    assert 0 < chain.address_count(blocksci.address_type.witness_pubkeyhash)
    assert 0 < chain.address_count(blocksci.address_type.witness_scripthash)
    assert 0 < chain.address_count(blocksci.address_type.nulldata)
    assert 0 < chain.address_count(blocksci.address_type.multisig)


def test_empty_block(chain, regtest):
    height = 90
    block_regression(chain[height], regtest)


def test_block_with_p2pkh_tx(chain, json_data, regtest):
    height = json_data['address-p2pkh-spend-1-height']
    block_regression(chain[height], regtest)


@pytest.mark.btc
def test_block_with_p2wsh_tx(chain, json_data, regtest):
    height = json_data['address-p2wsh-spend-2-height']
    block_regression(chain[height], regtest)


def block_regression(block, regtest):
    print(block.to_list(), file=regtest)
    print(block.base_size, file=regtest)
    print(block.bits, file=regtest)
    print(block.coinbase_param, file=regtest)
    print(block.coinbase_tx, file=regtest)
    print(block.fee, file=regtest)
    print(block.hash, file=regtest)
    print(block.height, file=regtest)
    print(block.input_count, file=regtest)
    print(block.input_value, file=regtest)
    print(block.miner(), file=regtest)
    print(sorted(block.net_address_type_value().items()), file=regtest)
    print(sorted(block.net_full_type_value().items()), file=regtest)
    print(block.next_block, file=regtest)
    print(block.nonce, file=regtest)
    print(block.output_count, file=regtest)
    print(block.output_value, file=regtest)
    print(block.prev_block, file=regtest)
    print(block.revenue, file=regtest)
    print(block.size_bytes, file=regtest)
    print(correct_timestamp(block.time), file=regtest)
    print(block.time_seen, file=regtest)
    print(block.timestamp, file=regtest)
    print(block.total_size, file=regtest)
    print(block.tx_count, file=regtest)
    print(block.txes, file=regtest)
    print(block.version, file=regtest)
    print(block.virtual_size, file=regtest)
    print(block.weight, file=regtest)
