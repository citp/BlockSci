cd test
blocksci_parser btc.json generate-config bitcoin_regtest bitcoin_regtest --disk files/btc/regtest/
blocksci_parser btc.json update
blocksci_parser bch.json generate-config bitcoin_cash_regtest bitcoin_cash_regtest --disk files/bch/regtest/
blocksci_parser bch.json update
