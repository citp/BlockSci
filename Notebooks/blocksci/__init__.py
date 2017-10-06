from blocksci.blocksci_interface import *
from blocksci.currency import *
from blocksci.blockchain_info import *
from blocksci.blocktrail import *
from blocksci.opreturn import *

from multiprocess import Pool
from functools import reduce
import operator
import datetime
import dateparser
from dateutil.relativedelta import relativedelta
import pandas as pd
import psutil
import tempfile
import importlib
import subprocess
import sys
import os
import inspect

def mapreduce_block_ranges(chain, mapFunc, reduceFunc, init,  start=None, end=None, cpu_count=psutil.cpu_count()):
    """Initialized multithreaded map reduce function over a stream of block ranges
    """
    if start is None:
        start = 0
        if end is None:
            end = len(chain)
    elif isinstance(start, str):
        blocks = chain.range(start, end)
        start = blocks[0].height
        end = blocks[-1].height

    if cpu_count == 1:
        return reduce(reduceFunc, (mapFunc(block) for block in chain[start:end]))

    segments = chain.segment(start, end, cpu_count)
    with Pool(cpu_count - 1) as p:
        results_future = p.map_async(mapFunc, segments[1:])
        last = mapFunc(segments[0])
        results = results_future.get()
        results.insert(0, last)
    return reduce(reduceFunc, results, init)


def mapreduce_blocks(chain, mapFunc, reduceFunc, init, start=None, end=None, cpu_count=psutil.cpu_count()):
    """Initialized multithreaded map reduce function over a stream of blocks
    """
    def mapRangeFunc(blocks):
        return reduce(reduceFunc, (mapFunc(block) for block in blocks), init)
    return mapreduce_block_ranges(chain, mapRangeFunc, reduceFunc, init, start, end, cpu_count)

def mapreduce_txes(chain, mapFunc, reduceFunc, init,  start=None, end=None, cpu_count=psutil.cpu_count()):
    """Initialized multithreaded map reduce function over a stream of transactions
    """
    def mapRangeFunc(blocks):
        return reduce(reduceFunc, (mapFunc(tx) for block in blocks for tx in block))
    return mapreduce_block_ranges(chain, mapRangeFunc, reduceFunc, init, start, end, cpu_count)


def map_blocks(self, blockFunc, start = None, end = None, cpu_count=psutil.cpu_count()):
    """Runs the given function over each block in range and returns a list of the results
    """
    def mapFunc(blocks):
        return [blockFunc(block) for block in blocks]

    return mapreduce_block_ranges(self, mapFunc, operator.concat, list(), start, end, cpu_count)

def filter_blocks(self, filterFunc, start = None, end = None, cpu_count=psutil.cpu_count()):
    """Return all blocks in range which match the given criteria
    """
    def mapFunc(blocks):
        return [block for block in blocks if filterFunc(block)]
    return  mapreduce_block_ranges(self, mapFunc, operator.concat, list(), start, end, cpu_count=cpu_count)

def filter_txes(self, filterFunc, start = None, end = None, cpu_count=psutil.cpu_count()):
    """Return all transactions in range which match the given criteria
    """
    def mapFunc(blocks):
        return [tx.index for block in blocks for tx in block if filterFunc(tx)]
    def reduceFunc(cur, el):
        return cur + [el]
    init = list()
    tx_ids = mapreduce_block_ranges(self, mapFunc, reduceFunc, init, start, end, cpu_count)
    return [Tx.tx_with_index(x) for x in tx_ids]

Blockchain.map_blocks = map_blocks
Blockchain.filter_blocks = filter_blocks
Blockchain.filter_txes = filter_txes
Blockchain.mapreduce_block_ranges = mapreduce_block_ranges
Blockchain.mapreduce_blocks = mapreduce_blocks
Blockchain.mapreduce_txes = mapreduce_txes

def heights_to_dates(self, df):
    return df.set_index(df.index.to_series().apply(lambda x: self[x].time))

def block_range(self, start, end=None):
    if self.block_times is None:
        self.block_times = pd.DataFrame([block.time for block in self], columns=["date"])
        self.block_times["height"] = self.block_times.index
        self.block_times.index = self.block_times["date"]
        del self.block_times["date"]

    start_date = pd.to_datetime(start)
    if end is None:
        res = dateparser.DateDataParser().get_date_data(start)
        if res['period'] == 'month':
            end = start_date + relativedelta(months=1)
        elif res['period'] == 'day':
            end = start_date + relativedelta(days=1)
        elif res['period'] == 'year':
            end = start_date + relativedelta(years=1)
    else:
        end = pd.to_datetime(end)

    return [self[height] for height in self.block_times[(self.block_times.index >= start_date) & (self.block_times.index < end)].height.tolist()]

old_init = Blockchain.__init__
def new_init(self, loc):
    old_init(self, loc)
    self.block_times = None
    self.cpp = CPP(self)
Blockchain.__init__ = new_init
Blockchain.range = block_range
Blockchain.heights_to_dates = heights_to_dates

class DummyClass:
    pass

loaderDirectory = os.path.dirname(os.path.abspath(inspect.getsourcefile(DummyClass)))


class CPP(object):
    def __init__(self, chain):
        self.dynamicFunctionCounter = 0
        self.module_directory = tempfile.TemporaryDirectory()
        sys.path.append(self.module_directory.name)
        self.saved_tx_filters = {}
        self.chain = chain

    def generate_module_name(self):
        module_name = "dynamicCode" + str(self.dynamicFunctionCounter)
        self.dynamicFunctionCounter += 1
        return module_name

    def filter_tx(self, code, start=None, end=None):
        if start is None:
            start = 0
        if end is None:
            end = len(self.chain)
        if code not in self.saved_tx_filters:
            from string import Template
            filein = open(loaderDirectory + '/filterTxesExtension.cpp')
            template = Template(filein.read())
            module_name = self.generate_module_name()
            filled_template = template.safe_substitute({"module_name":module_name, "func_def" : code})
            makefile = self.create_makefile(module_name)
            func = self.build_function(filled_template, makefile, module_name)
            self.saved_tx_filters[code] = func
        return self.saved_tx_filters[code](self.chain, start, end)

    def create_makefile(self, module_name):
        from string import Template
        import os
        filein = open(loaderDirectory + '/templateMakefile')
        template = Template(filein.read())
        subs = {"module_name" : module_name, "install_location" : self.module_directory.name, "srcname" : module_name + ".cpp", "loaderDirectory":loaderDirectory}
        return template.safe_substitute(subs)

    def build_function(self, full_code, makefile, module_name):
        builddir = tempfile.TemporaryDirectory()
        with open(builddir.name + '/' + module_name + ".cpp", 'w') as f:
            f.write(full_code)
        with open(builddir.name + '/CMakeLists.txt', 'w') as f:
            f.write(makefile)
        process = subprocess.Popen(["cmake", "."], cwd=builddir.name, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = process.communicate()
        print(err.decode('utf8'))
        process = subprocess.Popen(["make"], cwd=builddir.name, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = process.communicate()
        print(err.decode('utf8'))
        process = subprocess.Popen(["make", "install"], cwd=builddir.name, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
        out, err = process.communicate()
        print(err.decode('utf8'))
        mod = importlib.import_module(module_name)
        return getattr(mod, "func")