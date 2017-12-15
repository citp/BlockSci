export CFLAGS="$CFLAGS -I$PREFIX/include"
export CXXFLAGS="$CXXFLAGS -I$PREFIX/include"
mkdir -p build
cd build
cmake -LAH -DCMAKE_BUILD_TYPE="Release"  -DCMAKE_INSTALL_PREFIX=${PREFIX} -DCMAKE_PREFIX_PATH=${PREFIX} ..
 
make install -j${CPU_COUNT}
