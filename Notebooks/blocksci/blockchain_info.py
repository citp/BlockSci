import requests
import pandas as pd

class BlockchainInfoData(object):
    def __init__(self, api_key, cache_blocks=False, cache_txs=False):
        self.base_url = 'https://blockchain.info/'
        self.api_key = api_key
        self.cache_blocks = cache_blocks
        self.cache_txs = cache_txs
        self.block_cache = {}
        self.tx_cache = {}
        
    def block_data(self, block_hash):
        if block_hash in self.block_cache:
            return self.block_cache[block_hash]       
        
        url = self.base_url + 'rawblock/' + block_hash + "?api_code=" + self.api_key
        r = requests.get(url)
        r.raise_for_status()
        
        data = r.json()
        if self.cache_blocks:
            self.block_cache[block_hash] = data
        return data
    
    def tx_data(self, tx_hash):
        if tx_hash in self.tx_cache:
            return self.tx_cache[tx_hash]

        url = self.base_url + 'rawtx/' + tx_hash + "?api_code=" + self.api_key
        r = requests.get(url)
        r.raise_for_status()
        
        data = r.json()
        if self.cache_txs:
            self.tx_cache[tx_hash] = data
        return data
    
    def block_arrival_time(self, block_hash):
        data = self.block_data(block_hash)
        
        if "received_time" in data:
            return pd.to_datetime(data["received_time"], unit='s')
        else:
            return None
    
    def tx_arrival_times(self, block_hash):
        data = self.block_data(block_hash)
        arrival_times = {}
        for tx in data["tx"]:
            arrival_times[tx["hash"]] = pd.to_datetime(tx["time"], unit='s')
        return arrival_times
    
    def tx_arrival_time(self, tx_hash):
        data = self.tx_data(tx_hash)
        if "time" in data:
            return pd.to_datetime(data["time"], unit="s")
        else:
            return None
    
    def block_relayed_by(self, block_hash):
        data = self.block_data(block_hash)
        if "relayed_by" in data:
            return data["relayed_by"]
        else:
            return None