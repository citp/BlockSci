sed -i 's/j4/j2/g' blockscipy/setup.py
while sleep 240; do echo "    [    ] $SECONDS seconds still running"; done &
CC=gcc-7 CXX=g++-7 sudo -HE env PATH=$PATH PYTHONPATH=$PYTHONPATH pip3 install -v -e blockscipy
kill %1
