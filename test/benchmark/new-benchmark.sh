#!/bin/bash
pytest --benchmark-autosave --benchmark-warmup=true --benchmark-warmup-iterations=1 --benchmark-sort=name --benchmark-columns=mean,median,max,rounds,iterations --btc
