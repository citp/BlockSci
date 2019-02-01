cd "$TRAVIS_BUILD_DIR"
cd docs
python3 -msphinx -M html . _output -a -E
cd "$TRAVIS_BUILD_DIR"
