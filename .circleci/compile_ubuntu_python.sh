set -ev
sed -i 's/j4/j2/g' blockscipy/setup.py
CC=gcc-7 CXX=g++-7 pip3 install -v -e blockscipy
