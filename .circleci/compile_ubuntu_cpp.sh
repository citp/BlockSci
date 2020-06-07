set -ev
which cmake
cmake -version
mkdir -p release
cd release
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
mtime_cache ../**/*.{%{cpp}} -c ../.mtime_cache/cache.json
make -j 2
make install
make blocksci_unittest -j 2
mtime_cache /home/circleci/.local/**/*.{%{cpp}} -c ../.mtime_cache/blocksci.json
cd ..
