#!/bin/bash
pytest --benchmark-compare --benchmark-compare-fail=mean:5% --benchmark-warmup=true --benchmark-warmup-iterations=1 --benchmark-sort=name --benchmark-columns=mean,median,max,rounds,iterations --btc
