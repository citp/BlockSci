import os
import subprocess
import time
import shutil

chains = {
    "btc": "bitcoin_regtest",
    "bch": "bitcoin_cash_regtest",
    "ltc": "litecoin_regtest"
}

for _ in range(100):
    for chain_name in chains:
        self_dir = os.path.dirname(os.path.realpath(__file__))
        data_dir = self_dir + "/data"

        if not os.path.exists(data_dir):
            os.makedirs(data_dir)

        chain_dir = data_dir + "/" + chain_name
        config = chain_dir + "/config.json"

        create_config_cmd = [
            "blocksci_parser",
            config,
            "generate-config",
            chains[chain_name],
            chain_dir,
            "--disk",
            "{}/../files/{}/regtest/".format(self_dir, chain_name),
        ]

        parse_cmd = ["blocksci_parser", config, "update"]

        check_integrity_cmd = ["blocksci_check_integrity", config]

        subprocess.run(create_config_cmd, check=True, stdout=subprocess.DEVNULL)
        subprocess.run(parse_cmd, check=True) #, stdout=subprocess.DEVNULL)

        check_file = chain_dir + "/checksums.txt"
        with open(check_file, "w") as f:
            subprocess.run(check_integrity_cmd, check=True, stdout=f)

        reference_file = "{}/../files/{}/checksums.txt".format(self_dir, chain_name)

        files_are_same = True
        with open(check_file) as check, open(reference_file) as ref:
            for check_line, ref_line in zip(check, ref):
                if check_line != ref_line:
                    print("Checksums are different!")
                    print("Reference: {}".format(ref_line))
                    print("Parsing:   {}".format(check_line))
                    print()
                    files_are_same = False

        if not files_are_same:
            exit(1)
        else:
            print("Parsing successful. Deleting output directory.")
            shutil.rmtree(chain_dir)

        time.sleep(2)

