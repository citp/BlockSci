set -ev
cd test
blocksci_parser btc.json generate-config bitcoin_regtest bitcoin_regtest --disk files/btc/regtest/
blocksci_parser btc.json doctor
blocksci_parser btc.json update
blocksci_check_integrity btc.json -t -n
blocksci_parser bch.json generate-config bitcoin_cash_regtest bitcoin_cash_regtest --disk files/bch/regtest/
blocksci_parser bch.json doctor
blocksci_parser bch.json update
blocksci_check_integrity bch.json -t -n
blocksci_parser ltc.json generate-config litecoin_regtest litecoin_regtest --disk files/ltc/regtest/
blocksci_parser ltc.json doctor
blocksci_parser ltc.json update
blocksci_check_integrity ltc.json -t -n

./../release/test/blocksci/blocksci_unittest btc.json --gtest_output="xml:test-results/googletest.xml"

cd ..
