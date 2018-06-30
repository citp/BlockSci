import requests
import pandas as pd

class BlocktrailData(object):
    
    def __init__(self, api_key):
        self.api_key = api_key
        self.base_url = "https://api.blocktrail.com/v1/btc/"
        self.arrival_times = {}
    
    def block_header_data(self, height):
        url = self.base_url + 'block/{}?api_key={}'.format(height, self.api_key)
        r = requests.get(url)
        r.raise_for_status()
        return r.json()
        
    def block_arrival_time(self, height):
        if height in self.arrival_times:
            return self.arrival_times[height]
        ts = pd.to_datetime(self.block_header_data(height)['arrival_time'])
        self.arrival_times[height] = ts
        return ts    
    