mkdir -p release
cd release
CC=gcc-7 CXX=g++-7 cmake -DCMAKE_BUILD_TYPE=Release ..
mtime_cache ../**/*.{%{cpp}} -c ../.mtime_cache/cache.json
make -j 2
sudo make install
cd ..
