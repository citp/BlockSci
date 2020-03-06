import pytest
import subprocess
import os


def pytest_addoption(parser):
    parser.addoption("--btc", action="store_true", help="Run tests for Bitcoin")
    parser.addoption("--local", action="store", default="default", help="Run tests against local chain (useful for benchmark)")


def pytest_generate_tests(metafunc):
    metafunc.fixturenames.append("chain_name")
    chains = ["btc"]
    config_name = [metafunc.config.option.local]
    if metafunc.config.option.local != "default":
        chains = ["local"]
    metafunc.parametrize("chain_name", chains, scope="session")
    metafunc.parametrize("config", config_name, scope="session")


def pytest_runtest_call(item):
    markers = [x.name for x in item.iter_markers()]
    if markers:
        if item.funcargs["chain_name"] not in markers:
            pytest.skip(
                "Skipping test for chain {}".format(item.funcargs["chain_name"])
            )


@pytest.fixture(scope="session")
def chain(tmpdir_factory, chain_name, config):
    import blocksci

    if chain_name == "local":
        return blocksci.Blockchain(config)

    temp_dir = tmpdir_factory.mktemp(chain_name)
    chain_dir = str(temp_dir)
    self_dir = os.path.dirname(os.path.realpath(__file__))

    create_config_cmd = [
        "blocksci_parser",
        chain_dir + "/config.json",
        "generate-config",
        "bitcoin_regtest",
        chain_dir,
        "--disk",
        "{}/../files/{}/regtest/".format(self_dir, chain_name)
    ]
    parse_cmd = ["blocksci_parser", chain_dir + "/config.json", "update"]

    # Parse the chain
    subprocess.run(create_config_cmd, check=True)
    subprocess.run(parse_cmd, check=True)

    chain = blocksci.Blockchain(chain_dir + "/config.json")
    return chain