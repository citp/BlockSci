import pytest
import subprocess
import os


@pytest.fixture(scope="session")
def chain(tmpdir_factory):
    temp_dir = tmpdir_factory.mktemp("bitcoin_regtest")
    chain_dir = str(temp_dir)
    if not os.path.exists(chain_dir):
        os.mkdir(chain_dir)
    create_config_cmd = ["blocksci_parser", chain_dir + "/config.json", "generate-config", "bitcoin_regtest", chain_dir,
                         "--disk", "../files/regtest/"]
    subprocess.run(create_config_cmd, check=True)

    parse_cmd = ["blocksci_parser", chain_dir + "/config.json", "update"]
    subprocess.run(parse_cmd, check=True)

    import blocksci
    chain = blocksci.Blockchain(chain_dir + "/config.json")
    return chain


@pytest.fixture
def json_data():
    import json
    with open("../files/output.json", "r") as f:
        return json.load(f)
