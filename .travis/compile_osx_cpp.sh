cd "$TRAVIS_BUILD_DIR"
mkdir -p release
cd release
cmake -DCMAKE_BUILD_TYPE=Release -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl -DCMAKE_INSTALL_PREFIX=$HOME/.local ..
mtime_cache ../**/*.{%{cpp}} -c ../.mtime_cache/cache_osx.json
make -j 2
make install
mtime_cache "$HOME"/.local/**/*.{%{cpp}} -c "$TRAVIS_BUILD_DIR"/.mtime_cache/blocksci_osx.json
cd "$TRAVIS_BUILD_DIR"
