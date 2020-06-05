import pytest


@pytest.mark.bch
def test_ctor(chain, json_data):
    block = chain[json_data["bitcoin-cash-test-block"]]
    txes = [str(x.hash) for x in block.txes[1:]]
    assert txes == sorted(txes)
