set -ev
sudo apt install python3-dev python3-pip
sudo pip3 install -U pip setuptools
sudo pip3 install pytest pytest-regtest pytz tzlocal pytest-benchmark
sudo pip3 install sphinx sphinx_rtd_theme
