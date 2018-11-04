mkdir -p release
cd release
cmake -DCMAKE_BUILD_TYPE=Release -DOPENSSL_ROOT_DIR=/usr/local/opt/openssl ..
mtime_cache ../**/*.{%{cpp}} -c ../.mtime_cache/cache.json
make -j 2
sudo make install
cd ..
