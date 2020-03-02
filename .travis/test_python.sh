set -ev
cd "$TRAVIS_BUILD_DIR"
cd test/blockscipy
pytest -v
cd "$TRAVIS_BUILD_DIR"
cd test/benchmark
./new-benchmark.sh
cd "$TRAVIS_BUILD_DIR"
