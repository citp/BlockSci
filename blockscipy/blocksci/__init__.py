# -*- coding: utf-8 -*-
"""BlockSci Module

BlockSci enables fast and expressive analysis of Bitcoin’s and many
other blockchains.
"""

import tempfile
import importlib
import subprocess
import sys
import os
import inspect
import copy
import io
import re
from functools import reduce

import psutil
from multiprocess import Pool
import dateparser
from dateutil.relativedelta import relativedelta
import pandas as pd

from ._blocksci import *
from .currency import *
from .blockchain_info import *
from .opreturn import label_application
from .pickler import *

VERSION = "0.6.0"


sys.modules['blocksci.proxy'] = proxy
sys.modules['blocksci.cluster'] = cluster
sys.modules['blocksci.heuristics'] = heuristics
sys.modules['blocksci.heuristics.change'] = heuristics.change


class _NoDefault:
    def __repr__(self):
        return '(no default)'


MISSING_PARAM = _NoDefault()


def mapreduce_block_ranges(chain, map_func, reduce_func, init=MISSING_PARAM, start=None, end=None, cpu_count=psutil.cpu_count()):
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
        return mapFunc(chain[start:end])

    raw_segments = chain._segment_indexes(start, end, cpu_count)
    segments = [(raw_segment, chain.data_location, len(chain)) for raw_segment in raw_segments]

    def real_map_func(input):
        local_chain = Blockchain(input[1], input[2])
        file = io.BytesIO()
        pickler = Pickler(file)
        mapped = map_func(local_chain[input[0][0]:input[0][1]])
        pickler.dump(mapped)
        file.seek(0)
        return file

    with Pool(cpu_count - 1) as p:
        results_future = p.map_async(real_map_func, segments[1:])
        first = map_func(chain[raw_segments[0][0]:raw_segments[0][1]])
        results = results_future.get()
        results = [Unpickler(res, chain).load() for res in results]
    results.insert(0, first)
    if isinstance(init, type(MISSING_PARAM)):
        return reduce(reduce_func, results)
    else:
        return reduce(reduce_func, results, init)


def mapreduce_blocks(chain, map_func, reduce_func, init=MISSING_PARAM, start=None, end=None, cpu_count=psutil.cpu_count()):
    """Initialized multithreaded map reduce function over a stream of blocks
    """
    def map_range_func(blocks):
        if isinstance(init, type(MISSING_PARAM)):
            return reduce(reduce_func, (map_func(block) for block in blocks))
        else:
            return reduce(
                reduce_func,
                (map_func(block) for block in blocks),
                copy.deepcopy(init)
            )
    return mapreduce_block_ranges(
        chain,
        map_range_func,
        reduce_func,
        init,
        start,
        end,
        cpu_count
    )


def mapreduce_txes(chain, map_func, reduce_func, init=MISSING_PARAM, start=None, end=None, cpu_count=psutil.cpu_count()):
    """Initialized multithreaded map reduce function over a stream of transactions
    """
    def map_range_func(blocks):
        if isinstance(init, type(MISSING_PARAM)):
            return reduce(
                reduce_func,
                (map_func(tx) for block in blocks for tx in block)
            )
        else:
            return reduce(
                reduce_func,
                (map_func(tx) for block in blocks for tx in block),
                copy.deepcopy(init)
            )
    return mapreduce_block_ranges(
        chain,
        map_range_func,
        reduce_func,
        init,
        start,
        end,
        cpu_count
    )


def map_blocks(self, block_func, start=None, end=None, cpu_count=psutil.cpu_count()):
    """Runs the given function over each block in range and returns a list of the results
    """
    def map_func(blocks):
        return [block_func(block) for block in blocks]

    def reduce_func(accum, new_val):
        accum.extend(new_val)
        return accum

    return mapreduce_block_ranges(
        self,
        map_func,
        reduce_func,
        MISSING_PARAM,
        start,
        end,
        cpu_count
    )


def filter_blocks(self, filter_func, start=None, end=None, cpu_count=psutil.cpu_count()):
    """Return all blocks in range which match the given criteria
    """
    def map_func(blocks):
        return [block for block in blocks if filter_func(block)]

    def reduce_func(accum, new_val):
        accum.extend(new_val)
        return accum
    return mapreduce_block_ranges(
        self,
        map_func,
        reduce_func,
        MISSING_PARAM,
        start,
        end,
        cpu_count=cpu_count
    )


def filter_txes(self, filter_func, start=None, end=None, cpu_count=psutil.cpu_count()):
    """Return all transactions in range which match the given criteria
    """
    def map_func(blocks):
        return [tx for block in blocks for tx in block if filter_func(tx)]

    def reduce_func(accum, new_val):
        accum.extend(new_val)
        return accum
    return mapreduce_block_ranges(
        self,
        map_func,
        reduce_func,
        MISSING_PARAM,
        start,
        end,
        cpu_count
    )


Blockchain.map_blocks = map_blocks
Blockchain.filter_blocks = filter_blocks
Blockchain.filter_txes = filter_txes
Blockchain.mapreduce_block_ranges = mapreduce_block_ranges
Blockchain.mapreduce_blocks = mapreduce_blocks
Blockchain.mapreduce_txes = mapreduce_txes


def heights_to_dates(self, df):
    """
    Convert a pandas data frame with a block height index into a frame with a block time index
    """
    return df.set_index(df.index.to_series().apply(lambda x: self[x].time))


def block_range(self, start, end=None) -> BlockRange:
    """
    Return the range of blocks mined between the given dates
    """
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

    oldest = self.block_times[self.block_times.index >= start_date].iloc[0][0]
    newest = self.block_times[self.block_times.index <= end].iloc[-1][0] + 1

    return self[oldest:newest]


old_init = Blockchain.__init__


def new_init(self, loc, max_block=0):
    if max_block == 0:
        old_init(self, loc)
    else:
        old_init(self, loc, max_block)
    self.block_times = None
    self.cpp = CPP(self)
    ec2_instance_path = "/home/ubuntu/BlockSci/IS_EC2"
    tx_heated_path = "/home/ubuntu/BlockSci/TX_DATA_HEATED"
    scripts_heated_path = "/home/ubuntu/BlockSci/SCRIPT_DATA_HEATED"
    index_heated_path = "/home/ubuntu/BlockSci/INDEX_DATA_HEATED"

    if os.path.exists(ec2_instance_path):
        if not os.path.exists(tx_heated_path):
            print("Note: this appears to be a fresh instance. Transaction data has not yet been cached locally. Most queries might be slow. Caching is currently ongoing in the background, and usually takes 20 minutes.")
        elif not os.path.exists(scripts_heated_path):
            print("Note: this appears to be a fresh instance. Script data has not yet been cached locally. Some queries might be slow. Caching is currently ongoing in the background, and usually takes 1.5 hours.")
        elif not os.path.exists(index_heated_path):
            print("Note: this appears to be a fresh instance. Index data has not yet been cached locally. A few queries might be slow. Caching is currently ongoing in the background, and usually takes 3.5 hours.")


Blockchain.__init__ = new_init
Blockchain.range = block_range
Blockchain.heights_to_dates = heights_to_dates



def setup_map_funcs():
    def range_map_func(r, func):
        p = func(r.self_proxy.nested_proxy)
        return r.self_proxy._map(p)(r)

    def range_where_func(r, func):
        p = func(r.self_proxy.nested_proxy)
        return r.self_proxy._where(p)(r)

    def range_any_func(r, func):
        p = func(r.self_proxy.nested_proxy)
        return r.self_proxy._any(p)(r)

    def range_all_func(r, func):
        p = func(r.self_proxy.nested_proxy)
        return r.self_proxy._all(p)(r)

    def range_group_by_func(r, grouper_func, evaler_func):
        grouper = grouper_func(r.self_proxy.nested_proxy)
        evaler = evaler_func(r.self_proxy.nested_proxy.range_proxy)
        return r._group_by(grouper, evaler)

    iterator_and_range_cls = [x for x in globals() if ('Iterator' in x or 'Range' in x) and x[0].isupper()]

    for cl in iterator_and_range_cls:
        globals()[cl].map = range_map_func
        globals()[cl].where = range_where_func
        globals()[cl].group_by = range_group_by_func
        globals()[cl].any = range_any_func
        globals()[cl].all = range_all_func

def setup_proxy_map_funcs():
    def range_map_func(r, func):
        p = func(r.nested_proxy)
        return r._map(p)

    def range_where_func(r, func):
        p = func(r.nested_proxy)
        return r._where(p)

    def range_any_func(r, func):
        p = func(r.nested_proxy)
        return r._any(p)

    def range_all_func(r, func):
        p = func(r.nested_proxy)
        return r._all(p)

    iterator_and_range_cls = [x for x in dir(proxy) if ('Iterator' in x or 'Range' in x) and x[0].isupper()]

    for cl in iterator_and_range_cls:
        getattr(proxy, cl).map = range_map_func
        getattr(proxy, cl).where = range_where_func
        getattr(proxy, cl).any = range_any_func
        getattr(proxy, cl).all = range_all_func


setup_proxy_map_funcs()
setup_map_funcs()


def _get_functions_methods(obj):
    return (attr for attr in dir(obj) if
            not attr[:2] == '__' and
            not isinstance(getattr(type(obj), attr, None), property))


def _get_properties_methods(obj):
    return (attr for attr in dir(obj) if
            isinstance(getattr(type(obj), attr, None), property))


def setup_self_methods(main):
    proxy_obj = main.self_proxy

    def self_property_creator(name):
        return property(getattr(proxy_obj, name))

    def self_method_creator(name):
        return lambda main, *args: getattr(proxy_obj, name)(*args)(main)

    for proxy_func in _get_properties_methods(proxy_obj):
        setattr(main, proxy_func, self_property_creator(proxy_func))

    for proxy_func in _get_functions_methods(proxy_obj):
        setattr(main, proxy_func, self_method_creator(proxy_func))


def setup_iterator_methods(iterator):
    proxy_self = iterator.self_proxy
    proxy_obj = proxy_self.nested_proxy
    proxy_self_cl = type(proxy_self)
    

    def iterator_creator(name):
        return property(proxy_self._map(getattr(proxy_obj, name)))

    def iterator_method_creator(name):
        return lambda rng, *args: proxy_self._map(getattr(proxy_obj, name)(*args))(rng)

    def iterator_proxy_creator(name):
        return property(lambda self: self._map(getattr(proxy_obj, name)))

    def iterator_proxy_method_creator(name):
        return lambda self, *args: self._map(getattr(proxy_obj, name)(*args))

    for proxy_func in _get_properties_methods(proxy_obj):
        setattr(iterator, proxy_func, iterator_creator(proxy_func))
        setattr(proxy_self_cl, proxy_func, iterator_proxy_creator(proxy_func))

    for proxy_func in _get_functions_methods(proxy_obj):
        setattr(iterator, proxy_func, iterator_method_creator(proxy_func))
        setattr(proxy_self_cl, proxy_func, iterator_proxy_method_creator(proxy_func))


def setup_range_methods(blocksci_range):
    proxy_self = blocksci_range.self_proxy
    proxy_obj = proxy_self.nested_proxy
    proxy_self_cl = type(proxy_self)

    def range_creator(name):
        return property(proxy_self._map(getattr(proxy_obj, name)))

    def range_method_creator(name):
        return lambda rng, *args: proxy_self._map(getattr(proxy_obj, name)(*args))(rng)

    def range_proxy_creator(name):
        return property(lambda rng: rng._map(getattr(proxy_obj, name)))

    def range_proxy_method_creator(name):
        return lambda rng, *args: rng._map(getattr(proxy_obj, name)(*args))

    for proxy_func in _get_properties_methods(proxy_obj):
        setattr(blocksci_range, proxy_func, range_creator(proxy_func))
        setattr(proxy_self_cl, proxy_func, range_proxy_creator(proxy_func))

    for proxy_func in _get_functions_methods(proxy_obj):
        setattr(blocksci_range, proxy_func, range_method_creator(proxy_func))
        setattr(proxy_self_cl, proxy_func, range_proxy_method_creator(proxy_func))

    blocksci_range.__getitem__ = lambda rng, index: proxy_self[index](rng)

setup_self_methods(Block)
setup_self_methods(Tx)
setup_self_methods(Output)
setup_self_methods(Input)
setup_self_methods(EquivAddress)

setup_self_methods(PubkeyAddress)
setup_self_methods(PubkeyHashAddress)
setup_self_methods(WitnessPubkeyHashAddress)
setup_self_methods(MultisigPubkey)
setup_self_methods(ScriptHashAddress)
setup_self_methods(WitnessScriptHashAddress)
setup_self_methods(MultisigAddress)
setup_self_methods(NonStandardAddress)
setup_self_methods(OpReturn)

setup_self_methods(cluster.Cluster)
setup_self_methods(cluster.TaggedCluster)
setup_self_methods(cluster.TaggedAddress)

setup_iterator_methods(BlockIterator)
setup_iterator_methods(TxIterator)
setup_iterator_methods(OutputIterator)
setup_iterator_methods(InputIterator)
setup_iterator_methods(AddressIterator)
setup_iterator_methods(EquivAddressIterator)

setup_iterator_methods(PubkeyAddressIterator)
setup_iterator_methods(PubkeyHashAddressIterator)
setup_iterator_methods(WitnessPubkeyHashAddressIterator)
setup_iterator_methods(MultisigPubkeyIterator)
setup_iterator_methods(ScriptHashAddressIterator)
setup_iterator_methods(WitnessScriptHashAddressIterator)
setup_iterator_methods(MultisigAddressIterator)
setup_iterator_methods(NonstandardAddressIterator)
setup_iterator_methods(OpReturnIterator)

setup_iterator_methods(cluster.ClusterIterator)
setup_iterator_methods(cluster.TaggedClusterIterator)
setup_iterator_methods(cluster.TaggedAddressIterator)

setup_range_methods(BlockRange)
setup_range_methods(TxRange)
setup_range_methods(OutputRange)
setup_range_methods(InputRange)
setup_range_methods(AddressRange)
setup_range_methods(EquivAddressRange)

setup_range_methods(PubkeyAddressRange)
setup_range_methods(PubkeyHashAddressRange)
setup_range_methods(WitnessPubkeyHashAddressRange)
setup_range_methods(MultisigPubkeyRange)
setup_range_methods(ScriptHashAddressRange)
setup_range_methods(WitnessScriptHashAddressRange)
setup_range_methods(MultisigAddressRange)
setup_range_methods(NonstandardAddressRange)
setup_range_methods(OpReturnRange)

setup_range_methods(cluster.ClusterRange)
setup_range_methods(cluster.TaggedClusterRange)
setup_range_methods(cluster.TaggedAddressRange)


def txes_including_output_of_type(txes, typ):
    return txes.where(proxy.tx.outputs.any(proxy.output.address_type == typ))

TxIterator.including_output_of_type = txes_including_output_of_type

TxRange.including_output_of_type = txes_including_output_of_type


def inputs_sent_before_height(inputs, height):
    return inputs.where(lambda inp: inp.spent_tx.block.height < height)


def inputs_sent_after_height(inputs, height):
    return inputs.where(lambda inp: inp.spent_tx.block.height >= height)


def inputs_with_age_less_than(inputs, age):
    return inputs.where(lambda inp: inp.tx.block_height - inp.spent_tx.block.height < age)


def inputs_with_age_greater_than(inputs, age):
    return inputs.where(lambda inp: inp.tx.block_height - inp.spent_tx.block.height >= age)


def inputs_with_address_type(inputs, typ):
    return inputs.where(lambda inp: inp.address_type == typ)


InputIterator.sent_before_height = inputs_sent_before_height
InputIterator.sent_after_height = inputs_sent_after_height
InputIterator.with_age_less_than = inputs_with_age_less_than
InputIterator.with_age_greater_than = inputs_with_age_greater_than
InputIterator.with_address_type = inputs_with_address_type

InputRange.sent_before_height = inputs_sent_before_height
InputRange.sent_after_height = inputs_sent_after_height
InputRange.with_age_less_than = inputs_with_age_less_than
InputRange.with_age_greater_than = inputs_with_age_greater_than
InputRange.with_address_type = inputs_with_address_type

def _outputAge(output):
    return output.spending_tx.map(lambda tx: tx.block_height) - output.tx.block_height

def outputs_unspent(outputs, height = -1):
    if height == -1:
        return outputs.where(lambda output: ~output.is_spent)
    else:
        return outputs.where(lambda output: (~output.is_spent) | (output.spending_tx.block_height.or_value(0) > height))


def outputs_spent_before_height(outputs, height):
    return outputs.where(lambda output: output.is_spent).where(lambda output: output.spending_tx.map(lambda tx: tx.block_height).or_value(0) < height)


def outputs_spent_after_height(outputs, age):
    return outputs.where(lambda output: output.is_spent).where(lambda output: output.spending_tx.map(lambda tx: tx.block_height).or_value(0) >= height)


def outputs_spent_with_age_less_than(outputs, age):
    return outputs.where(lambda output: output.is_spent).where(lambda output: _outputAge(output).or_value(0) < age)

def outputs_spent_with_age_greater_than(outputs, age):
    return outputs.where(lambda output: output.is_spent).where(lambda output: _outputAge(output).or_value(0) >= age)

def outputs_with_address_type(outputs, typ):
    return outputs.where(lambda output: output.address_type == typ)


OutputIterator.unspent = outputs_unspent
OutputIterator.spent_before_height = outputs_spent_before_height
OutputIterator.spent_after_height = outputs_spent_after_height
OutputIterator.spent_with_age_less_than = outputs_spent_with_age_less_than
OutputIterator.outputs_spent_with_age_greater_than = outputs_spent_with_age_greater_than
OutputIterator.with_address_type = outputs_with_address_type

OutputRange.unspent = outputs_unspent
OutputRange.spent_before_height = outputs_spent_before_height
OutputRange.spent_after_height = outputs_spent_after_height
OutputRange.spent_with_age_less_than = outputs_spent_with_age_less_than
OutputRange.outputs_spent_with_age_greater_than = outputs_spent_with_age_greater_than
OutputRange.with_address_type = outputs_with_address_type

first_miner_run = True


class DummyClass:
    pass


loaderDirectory = os.path.dirname(os.path.abspath(inspect.getsourcefile(DummyClass)))


def get_miner(block) -> str:
    """
    Get the miner of the block based on the text in the coinbase transaction
    """
    global first_miner_run
    global tagged_addresses
    global pool_data
    global coinbase_tag_re
    if first_miner_run:
        import json
        with open(loaderDirectory + "/Blockchain-Known-Pools/pools.json") as f:
            pool_data = json.load(f)
        addresses = [block._access.address_from_string(addr_string) for addr_string in pool_data["payout_addresses"]]
        tagged_addresses = {pointer: pool_data["payout_addresses"][address] for address in addresses if address in pool_data["payout_addresses"]}
        coinbase_tag_re = re.compile('|'.join(map(re.escape, pool_data["coinbase_tags"])))
        first_miner_run = False
    coinbase = block.coinbase_param.decode("utf_8", "replace")
    tag_matches = re.findall(coinbase_tag_re, coinbase)
    if tag_matches:
        return pool_data["coinbase_tags"][tag_matches[0]]["name"]
    for txout in block.coinbase_tx.outs:
        if txout.address in tagged_addresses:
            return tagged_addresses[txout.address]["name"]
    
    additional_miners = {
        "EclipseMC": "EclipseMC",
        "poolserverj": "poolserverj",
        "/stratumPool/": "stratumPool",
        "/stratum/": "stratum",
        "/nodeStratum/": "nodeStratum",
        "BitLC": "BitLC",
        "/TangPool/": "TangPool",
        "/Tangpool/": "TangPool",
        "pool.mkalinin.ru": "pool.mkalinin.ru",
        "For Pierce and Paul": "Pierce and Paul",
        "50btc.com": "50btc.com",
        "七彩神仙鱼": "F2Pool"
    }

    for miner in additional_miners:
        if miner in coinbase:
            return additional_miners[miner]

    return "Unknown"


Block.miner = get_miner


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
            filled_template = template.safe_substitute({
                "module_name": module_name,
                "func_def": code
            })
            makefile = self.create_makefile(module_name)
            func = self.build_function(filled_template, makefile, module_name)
            self.saved_tx_filters[code] = func
        return self.saved_tx_filters[code](self.chain, start, end)

    def create_makefile(self, module_name):
        from string import Template
        filein = open(loaderDirectory + '/templateMakefile')
        template = Template(filein.read())
        subs = {
            "module_name": module_name,
            "install_location": self.module_directory.name,
            "srcname": module_name + ".cpp",
            "python_blocksci_dir": loaderDirectory
        }
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