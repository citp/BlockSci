import pytest
import subprocess
import os


@pytest.fixture(scope="session")
def chain(tmpdir_factory):
    temp_dir = tmpdir_factory.mktemp("bitcoin_regtest")
    chain_dir = str(temp_dir)
    if not os.path.exists(chain_dir):
        os.mkdir(chain_dir)
    parse_cmd = ["blocksci_parser", "--output-directory", chain_dir, "update", "disk", "--coin-directory",
                 "../files/regtest/"]
    subprocess.run(parse_cmd)

    import blocksci
    chain = blocksci.Blockchain(chain_dir)
    return chain


@pytest.fixture
def json_data():
    import json
    with open("../files/output.json", "r") as f:
        return json.load(f)
