import pytest


def positive_locktime(chain):
    return chain.txes.where(lambda tx: tx.locktime > 0).size


def test_query_positive_locktime_10(chain, benchmark):
    offset = min(len(chain), 10)
    benchmark(positive_locktime, chain[-offset:])


def test_query_positive_locktime_100(chain, benchmark):
    offset = min(len(chain), 100)
    benchmark(positive_locktime, chain[-offset:])


@pytest.mark.local
def test_query_positive_locktime_1k(chain, benchmark):
    offset = min(len(chain), 1000)
    benchmark(positive_locktime, chain[-offset:])


@pytest.mark.local
def test_query_positive_locktime_10k(chain, benchmark):
    offset = min(len(chain), 10000)
    benchmark(positive_locktime, chain[-offset:])


@pytest.mark.local
def test_query_positive_locktime_100k(chain, benchmark):
    offset = min(len(chain), 100000)
    benchmark(positive_locktime, chain[-offset:])


def max_output_value(chain):
    return chain.outputs.max(lambda o: o.value).value


def test_query_max_output_value_10(chain, benchmark):
    offset = min(len(chain), 10)
    benchmark(max_output_value, chain[-offset:])


def test_query_max_output_value_100(chain, benchmark):
    offset = min(len(chain), 100)
    benchmark(max_output_value, chain[-offset:])


@pytest.mark.local
def test_query_max_output_value_1k(chain, benchmark):
    offset = min(len(chain), 1000)
    benchmark(max_output_value, chain[-offset:])


@pytest.mark.local
def test_query_max_output_value_10k(chain, benchmark):
    offset = min(len(chain), 10000)
    benchmark(max_output_value, chain[-offset:])


@pytest.mark.local
def test_query_max_output_value_100k(chain, benchmark):
    offset = min(len(chain), 100000)
    benchmark(max_output_value, chain[-offset:])
