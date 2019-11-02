set -ev
sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
sudo apt-get update
sudo apt install gcc-7 g++-7 libtool autoconf libboost-filesystem-dev libboost-iostreams-dev libboost-serialization-dev libboost-thread-dev libboost-test-dev  libssl-dev libjsoncpp-dev libcurl4-openssl-dev libjsoncpp-dev libjsonrpccpp-dev libsnappy-dev zlib1g-dev libbz2-dev liblz4-dev libzstd-dev libjemalloc-dev libsparsehash-dev
sudo apt-get install ruby-full -y  # only needed for caching
gem install mtime_cache  # only needed for caching
