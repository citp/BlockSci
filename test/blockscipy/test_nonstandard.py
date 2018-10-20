"""
Tests a number of less common characteristics of blocks and transactions
(not necessarily nonstandard as in what Bitcoin Core defines as nonstandard scripts).
"""

from util import Coin


def test_reward_partially_claimed(chain, json_data):
    height = json_data['block-partial-reward-height']
    coinbase = chain[height].txes[0]
    reward = Coin(50 - 10) if height < 150 else Coin(25 - 10)
    assert reward == sum(output.value for output in coinbase.outputs)


def test_fees_not_claimed(chain, json_data):
    height = json_data['block-fee-unclaimed-height']
    coinbase = chain[height].txes[0]
    reward = Coin(50) if height < 150 else Coin(25)
    assert reward == sum(output.value for output in coinbase.outputs)

    # sanity check that there were actually fees to be claimed
    total_fees_in_block = sum(tx.fee for tx in chain[height].txes)
    assert 0 < total_fees_in_block


def test_positive_locktime(chain, json_data):
    tx = chain.tx_with_hash(json_data["change-locktime-tx-1"])
    assert 110 == tx.locktime


def test_non_max_nsequence_no(chain, json_data):
    tx = chain.tx_with_hash(json_data['nsequence-fffffffe-tx'])
    inpt = tx.inputs[0]
    assert 2 ** 32 - 2 == inpt.sequence_num


def test_op_return(chain, json_data):
    tx = chain.tx_with_hash(json_data['op-return-tx'])
    out = tx.outputs[0]
    op_return = tx.op_return

    assert op_return
    assert out == op_return
    assert b"Lord Voldemort" == op_return.address.data
