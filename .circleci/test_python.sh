set -ev
cd test
mkdir test-results
cd blockscipy
pytest -v --junitxml=../test-results/junit.xml
cd ../benchmark
./new-benchmark.sh
