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
import logging
import inspect
import copy
import io
import re
import heapq
import operator
import time
from functools import reduce

import psutil
from multiprocess import Pool
import dateparser
from dateutil.relativedelta import relativedelta
import pandas as pd

from ._blocksci import *
from ._blocksci import _traverse
from .currency import *
from .blockchain_info import *
from .opreturn import label_application
from .pickler import *

VERSION = "0.7.0"


sys.modules['blocksci.proxy'] = proxy
sys.modules['blocksci.cluster'] = cluster
sys.modules['blocksci.heuristics'] = heuristics
sys.modules['blocksci.heuristics.change'] = heuristics.change


class _NoDefault:
    def __repr__(self):
        return '(no default)'


MISSING_PARAM = _NoDefault()


# Alert the user if the disk space is getting full
disk_info = os.statvfs("/")
free_space = (disk_info.f_frsize * disk_info.f_bavail) // (1024**3)
if free_space < 20:
    logger = logging.getLogger()
    logger.warning("Warning: You only have {}GB of free disk space left. Running out of disk space may crash the parser and corrupt the BlockSci data files.".format(free_space))

# UTC time zone is recommended
if time.tzname != ('UTC', 'UTC'):
    logger = logging.getLogger()
    logger.warning("Warning: Your system is set to a timezone other than UTC, leading to inconsistencies between datetime objects (which are adjusted to your local timezone) and datetime64 timestamps returned by iterators and ranges, or the fluent interface (which use UTC).")



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
    segments = [(raw_segment, chain.config_location, len(chain)) for raw_segment in raw_segments]

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


def filter_blocks(
    self, filter_func, start=None, end=None, cpu_count=psutil.cpu_count()
):
    """Return all blocks in range which match the given criteria
    """

    def map_func(blocks):
        return blocks.where(filter_func).to_list()

    def reduce_func(accum, new_val):
        accum.extend(new_val)
        return accum

    return mapreduce_block_ranges(
        self, map_func, reduce_func, MISSING_PARAM, start, end, cpu_count=cpu_count
    )


def filter_blocks_legacy(
    self, filter_func, start=None, end=None, cpu_count=psutil.cpu_count()
):
    """Return all blocks in range which match the given criteria
    """

    def map_func(blocks):
        return [block for block in blocks if filter_func(block)]

    def reduce_func(accum, new_val):
        accum.extend(new_val)
        return accum

    return mapreduce_block_ranges(
        self, map_func, reduce_func, MISSING_PARAM, start, end, cpu_count=cpu_count
    )


def filter_txes(self, filter_func, start=None, end=None, cpu_count=psutil.cpu_count()):
    """Return all transactions in range which match the given criteria
    """

    def map_func(blocks):
        return blocks.txes.where(filter_func).to_list()

    def reduce_func(accum, new_val):
        accum.extend(new_val)
        return accum

    return mapreduce_block_ranges(
        self, map_func, reduce_func, MISSING_PARAM, start, end, cpu_count
    )


def filter_txes_legacy(
    self, filter_func, start=None, end=None, cpu_count=psutil.cpu_count()
):
    """Return all transactions in range which match the given criteria
    """

    def map_func(blocks):
        return [tx for block in blocks for tx in block if filter_func(tx)]

    def reduce_func(accum, new_val):
        accum.extend(new_val)
        return accum

    return mapreduce_block_ranges(
        self, map_func, reduce_func, MISSING_PARAM, start, end, cpu_count
    )


Blockchain.map_blocks = map_blocks
Blockchain.filter_blocks = filter_blocks
Blockchain.filter_blocks_legacy = filter_blocks_legacy
Blockchain.filter_txes = filter_txes
Blockchain.filter_txes_legacy = filter_txes_legacy
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


def most_valuable_addresses(self, nlargest=100):
    current_address_vals = self.blocks.outputs.where(lambda o: ~o.is_spent) \
    .group_by( \
        lambda output: output.address, \
        lambda outputs: outputs.value.sum \
    )
    return heapq.nlargest(nlargest, current_address_vals.items(), key=operator.itemgetter(1))

Blockchain.__init__ = new_init
Blockchain.range = block_range
Blockchain.heights_to_dates = heights_to_dates
Blockchain.most_valuable_addresses = most_valuable_addresses

def traverse(proxy_func, val):
    return _traverse(proxy_func(val._self_proxy), val)

def apply_map(prox, prop):
    if prop.ptype == proxy.proxy_type.optional:
        return prox._map_optional(prop)
    elif prop.ptype == proxy.proxy_type.iterator:
        return prox._map_sequence(prop)
    elif prop.ptype == proxy.proxy_type.range:
        return prox._map_sequence(prop)
    else:
        return prox._map(prop)

def setup_optional_proxy_map_funcs():
    def optional_map_func(r, func):
        p = func(r.nested_proxy)
        return r._map(p)

    optional_cls = [x for x in dir(proxy) if 'Optional' in x and x[0].isupper()]

    for cl in optional_cls:
        getattr(proxy, cl).map = optional_map_func

def setup_sequence_map_funcs():
    def range_map_func(r, func):
        p = func(r._self_proxy.nested_proxy)
        return apply_map(r._self_proxy, p)(r)

    def range_where_func(r, func):
        p = func(r._self_proxy.nested_proxy)
        return r._self_proxy._where(p)(r)

    def range_max_func(r, func):
        p = func(r._self_proxy.nested_proxy)
        return r._self_proxy._max(p)(r)

    def range_min_func(r, func):
        p = func(r._self_proxy.nested_proxy)
        return r._self_proxy._min(p)(r)

    def range_any_func(r, func):
        p = func(r._self_proxy.nested_proxy)
        return r._self_proxy._any(p)(r)

    def range_all_func(r, func):
        p = func(r._self_proxy.nested_proxy)
        return r._self_proxy._all(p)(r)

    def range_group_by_func(r, grouper_func, evaler_func):
        grouper = grouper_func(r._self_proxy.nested_proxy)
        evaler = evaler_func(r._self_proxy.nested_proxy.range_proxy)
        return r._group_by(grouper, evaler)

    iterator_and_range_cls = [x for x in globals() if ('Iterator' in x or 'Range' in x) and x[0].isupper()]

    for cl in iterator_and_range_cls:
        globals()[cl].map = range_map_func
        globals()[cl].select = range_map_func
        globals()[cl].where = range_where_func
        globals()[cl].group_by = range_group_by_func
        globals()[cl].max = range_max_func
        globals()[cl].min = range_min_func
        globals()[cl].any = range_any_func
        globals()[cl].all = range_all_func

def setup_sequence_proxy_map_funcs():
    def range_map_func(r, func):
        p = func(r.nested_proxy)
        return apply_map(r, p)

    def range_where_func(r, func):
        p = func(r.nested_proxy)
        return r._where(p)

    def range_max_func(r, func):
        p = func(r.nested_proxy)
        return r._max(p)

    def range_min_func(r, func):
        p = func(r.nested_proxy)
        return r._min(p)

    def range_any_func(r, func):
        p = func(r.nested_proxy)
        return r._any(p)

    def range_all_func(r, func):
        p = func(r.nested_proxy)
        return r._all(p)

    iterator_and_range_cls = [x for x in dir(proxy) if ('Iterator' in x or 'Range' in x) and x[0].isupper()]

    for cl in iterator_and_range_cls:
        getattr(proxy, cl).map = range_map_func
        getattr(proxy, cl).select = range_map_func
        getattr(proxy, cl).where = range_where_func
        getattr(proxy, cl).max = range_max_func
        getattr(proxy, cl).min = range_min_func
        getattr(proxy, cl).any = range_any_func
        getattr(proxy, cl).all = range_all_func

non_copying_methods = set(["ptype", "iterator_proxy", "range_proxy", "optional_proxy", "output_type_name"])

def _get_core_functions_methods(obj):
    return (attr for attr in obj.__dict__ if
            not attr[:2] == '__' and attr not in non_copying_methods and
            not isinstance(getattr(obj, attr, None), property))


def _get_core_properties_methods(obj):
    return (attr for attr in obj.__dict__ if attr not in non_copying_methods and
            isinstance(getattr(obj, attr, None), property))

def _get_functions_methods(obj):
    return (attr for attr in dir(obj) if
            not attr[:2] == '__' and attr not in non_copying_methods and
            not isinstance(getattr(obj, attr, None), property))


def _get_properties_methods(obj):
    return (attr for attr in dir(obj) if attr not in non_copying_methods and
            isinstance(getattr(obj, attr, None), property))

# https://gist.github.com/carlsmith/b2e6ba538ca6f58689b4c18f46fef11c
def replace(string, substitutions):
    substrings = sorted(substitutions, key=len, reverse=True)
    regex = re.compile('|'.join(map(re.escape, substrings)))
    return regex.sub(lambda match: substitutions[match.group(0)], string)

def fix_all_doc_def(doc):
    doc = replace(doc, {
        "blocksci.proxy.intIteratorProxy": "numpy.ndarray[int]",
        "blocksci.proxy.intRangeProxy": "numpy.ndarray[int]",
        "blocksci.proxy.boolIteratorProxy": "numpy.ndarray[bool]",
        "blocksci.proxy.boolRangeProxy": "numpy.ndarray[bool]",
        "blocksci.proxy.ClusterIteratorProxy": "blocksci.cluster.ClusterIterator",
        "blocksci.proxy.TaggedClusterIteratorProxy": "blocksci.cluster.TaggedClusterIterator",
        "blocksci.proxy.TaggedAddressIteratorProxy": "blocksci.cluster.TaggedAddressIterator"
    })
    doc = re.sub(r"(blocksci\.proxy\.)([a-zA-Z]+)(IteratorProxy)", r"blocksci.\2Iterator", doc)
    return doc

def fix_self_doc_def(doc):
    doc = fix_all_doc_def(doc)
    doc = replace(doc, {
        "blocksci.proxy.ProxyAddress": "blocksci.Address",
        "blocksci.proxy.intProxy": "int",
        "blocksci.proxy.boolProxy": "bool",
        "blocksci.proxy.ClusterProxy": "blocksci.cluster.Cluster",
        "blocksci.proxy.TaggedClusterProxy": "blocksci.cluster.TaggedCluster",
        "blocksci.proxy.TaggedAddressProxy": "blocksci.cluster.TaggedAddress",
        "blocksci.proxy.ClusterRangeProxy": "blocksci.cluster.ClusterRange",
        "blocksci.proxy.TaggedClusterRangeProxy": "blocksci.cluster.TaggedClusterRange",
        "blocksci.proxy.TaggedAddressRangeProxy": "blocksci.cluster.TaggedAddressRange"
    })

    doc = re.sub(r"(blocksci\.proxy\.Optional)([a-zA-Z]+)(Proxy)", r"Optional\[blocksci\.\2\]", doc)
    doc = re.sub(r"(blocksci\.proxy\.)([a-zA-Z]+)(RangeProxy)", r"blocksci.\2Range", doc)
    doc = re.sub(r"(blocksci\.proxy\.)([a-zA-Z]+)(Proxy)", r"blocksci.\2", doc)
    return doc

def fix_sequence_doc_def(doc):
    doc = fix_all_doc_def(doc)
    doc = replace(doc, {
        "blocksci.proxy.intProxy": "numpy.ndarray[int]",
        "blocksci.proxy.boolProxy": "numpy.ndarray[bool]",
        "blocksci.proxy.ClusterRangeProxy": "blocksci.cluster.ClusterIterator",
        "blocksci.proxy.TaggedClusterRangeProxy": "blocksci.cluster.TaggedClusterIterator",
        "blocksci.proxy.TaggedAddressRangeProxy": "blocksci.cluster.ClusterIterator"
    })

    doc = re.sub(r"(blocksci\.proxy\.Optional)([a-zA-Z]+)(Proxy)", r"blocksci\.\2Iterator", doc)
    doc = re.sub(r"(blocksci\.proxy\.)([a-zA-Z]+)(RangeProxy)", r"blocksci.\2Iterator", doc)
    return doc

def fix_iterator_doc_def(doc):
    doc = fix_sequence_doc_def(doc)
    doc = replace(doc, {
        "blocksci.proxy.ClusterProxy": "blocksci.cluster.ClusterIterator",
        "blocksci.proxy.TaggedClusterProxy": "blocksci.cluster.TaggedClusterIterator",
        "blocksci.proxy.TaggedAddressProxy": "blocksci.cluster.TaggedAddressIterator"
    })

    doc = re.sub(r"(blocksci\.proxy\.)([a-zA-Z]+)(Proxy)", r"blocksci\.\2Iterator", doc)
    return doc

def fix_range_doc_def(doc):
    doc = fix_sequence_doc_def(doc)
    doc = replace(doc, {
        "blocksci.proxy.ClusterProxy": "blocksci.cluster.ClusterRange",
        "blocksci.proxy.TaggedClusterProxy": "blocksci.cluster.TaggedClusterRange",
        "blocksci.proxy.TaggedAddressProxy": "blocksci.cluster.TaggedAddressRange"
    })

    doc = re.sub(r"(blocksci\.proxy\.)([a-zA-Z]+)(Proxy)", r"blocksci\.\2Range", doc)
    return doc

def setup_self_methods(main, proxy_obj_type=None, sample_proxy=None):
    if proxy_obj_type is None:
        proxy_obj_type = type(main._self_proxy)
    if sample_proxy is None:
        sample_proxy = main._self_proxy

    # ignore properties that already exist (normal pybind11 binding)
    existing_properties = set(dir(main))

    def self_property_creator(name):
        prop = property(lambda s: getattr(s._self_proxy, name)(s))
        prop.__doc__ = str(getattr(proxy_obj_type, name).__doc__) + "\n\n:type: :class:`" + getattr(sample_proxy, name).output_type_name + "`"
        return prop

    def self_method_creator(name):
        def method(s, *args):
            return getattr(s._self_proxy, name)(*args)(s)

        orig_doc = getattr(proxy_obj_type, name).__doc__
        split = orig_doc.split("\n\n")
        method.__doc__ = fix_self_doc_def(split[0]) + '\n\n' + split[1]
        return method

    core_properties_methods = set(_get_core_properties_methods(proxy_obj_type)) - existing_properties
    core_functions_methods = set(_get_core_functions_methods(proxy_obj_type)) - existing_properties

    for proxy_func in core_properties_methods:
        setattr(main, proxy_func, self_property_creator(proxy_func))

    for proxy_func in core_functions_methods:
        setattr(main, proxy_func, self_method_creator(proxy_func))


def setup_iterator_methods(iterator, doc_func=fix_iterator_doc_def, nested_proxy_cl=None, sample_proxy=None):
    if nested_proxy_cl is None:
        nested_proxy_cl = type(iterator._self_proxy.nested_proxy)
        sample_proxy = iterator._self_proxy

    def iterator_creator(name):
        def method(s):
            return apply_map(s._self_proxy, getattr(s._self_proxy.nested_proxy, name))(s)
        prop = property(method)
        prop.__doc__ = "For each item: " + \
                       getattr(nested_proxy_cl, name).__doc__ + \
                       "\n\n:type: :class:`" + \
                       apply_map(sample_proxy, getattr(sample_proxy.nested_proxy, name)).output_type_name + \
                       "`"
        return prop

    def iterator_method_creator(name):
        def method(rng, *args):
            return apply_map(rng._self_proxy, getattr(rng._self_proxy.nested_proxy, name)(*args))(rng)

        orig_doc = getattr(nested_proxy_cl, name).__doc__
        split = orig_doc.split("\n\n")
        if len(split) != 2:
            print(iterator, name)
        method.__doc__ = doc_func(split[0]) + '\n\nFor each item: ' + split[1]
        return method

    for proxy_func in _get_core_properties_methods(nested_proxy_cl):
        setattr(iterator, proxy_func, iterator_creator(proxy_func))

    for proxy_func in _get_core_functions_methods(nested_proxy_cl):
        setattr(iterator, proxy_func, iterator_method_creator(proxy_func))

def setup_iterator_proxy_methods(iterator_proxy):
    proxy_cl = type(iterator_proxy)
    nested_proxy_cl = type(iterator_proxy.nested_proxy)

    def iterator_proxy_creator(name):
        def method(rng):
            return apply_map(rng, getattr(rng.nested_proxy, name))
        return property(method)

    def iterator_proxy_method_creator(name):
        def method(rng, *args):
            return apply_map(rng, getattr(rng.nested_proxy, name)(*args))

        return method

    for proxy_func in _get_core_properties_methods(nested_proxy_cl):
        setattr(proxy_cl, proxy_func, iterator_proxy_creator(proxy_func))

    for proxy_func in _get_core_functions_methods(nested_proxy_cl):
        setattr(proxy_cl, proxy_func, iterator_proxy_method_creator(proxy_func))

def setup_size_property(iterator):
    iterator.count = property(lambda rng: rng._self_proxy.size(rng))
    iterator.size = property(lambda rng: rng._self_proxy.size(rng))


def setup_range_methods(blocksci_range, nested_proxy_cl=None, sample_proxy=None):
    setup_iterator_methods(blocksci_range, fix_range_doc_def, nested_proxy_cl, sample_proxy)
    blocksci_range.__getitem__ = lambda rng, index: rng._self_proxy[index](rng)

def setup_iterator_and_proxy_methods(iterator):
    setup_iterator_methods(iterator)
    setup_iterator_proxy_methods(iterator._self_proxy)
    setup_size_property(iterator)

def setup_range_and_proxy_methods(blocksci_range):
    setup_range_methods(blocksci_range)
    setup_iterator_proxy_methods(blocksci_range._self_proxy)
    setup_size_property(blocksci_range)

setup_optional_proxy_map_funcs()
setup_sequence_proxy_map_funcs()
setup_sequence_map_funcs()

setup_self_methods(Block)
setup_self_methods(Tx)
setup_self_methods(Output)
setup_self_methods(Input)
setup_self_methods(EquivAddress)

setup_self_methods(Address, proxy.ProxyAddress, PubkeyAddress._self_proxy)
setup_self_methods(PubkeyAddress)
setup_self_methods(PubkeyHashAddress)
setup_self_methods(WitnessPubkeyHashAddress)
setup_self_methods(MultisigPubkey)
setup_self_methods(ScriptHashAddress)
setup_self_methods(WitnessScriptHashAddress)
setup_self_methods(MultisigAddress)
setup_self_methods(NonStandardAddress)
setup_self_methods(OpReturn)
setup_self_methods(WitnessUnknownAddress)

setup_self_methods(cluster.Cluster)
setup_self_methods(cluster.TaggedCluster)
setup_self_methods(cluster.TaggedAddress)

setup_iterator_and_proxy_methods(BlockIterator)
setup_iterator_and_proxy_methods(TxIterator)
setup_iterator_and_proxy_methods(OutputIterator)
setup_iterator_and_proxy_methods(InputIterator)
setup_iterator_and_proxy_methods(AddressIterator)
setup_iterator_and_proxy_methods(EquivAddressIterator)

setup_iterator_methods(GenericAddressIterator, proxy.ProxyAddress, PubkeyAddressIterator._self_proxy)
setup_iterator_and_proxy_methods(PubkeyAddressIterator)
setup_iterator_and_proxy_methods(PubkeyHashAddressIterator)
setup_iterator_and_proxy_methods(WitnessPubkeyHashAddressIterator)
setup_iterator_and_proxy_methods(MultisigPubkeyIterator)
setup_iterator_and_proxy_methods(ScriptHashAddressIterator)
setup_iterator_and_proxy_methods(WitnessScriptHashAddressIterator)
setup_iterator_and_proxy_methods(MultisigAddressIterator)
setup_iterator_and_proxy_methods(NonstandardAddressIterator)
setup_iterator_and_proxy_methods(OpReturnIterator)

setup_iterator_and_proxy_methods(cluster.ClusterIterator)
setup_iterator_and_proxy_methods(cluster.TaggedClusterIterator)
setup_iterator_and_proxy_methods(cluster.TaggedAddressIterator)

setup_range_and_proxy_methods(BlockRange)
setup_range_and_proxy_methods(TxRange)
setup_range_and_proxy_methods(OutputRange)
setup_range_and_proxy_methods(InputRange)
setup_range_and_proxy_methods(AddressRange)
setup_range_and_proxy_methods(EquivAddressRange)

setup_range_methods(GenericAddressRange, proxy.ProxyAddress, PubkeyAddressRange._self_proxy)
setup_range_and_proxy_methods(PubkeyAddressRange)
setup_range_and_proxy_methods(PubkeyHashAddressRange)
setup_range_and_proxy_methods(WitnessPubkeyHashAddressRange)
setup_range_and_proxy_methods(MultisigPubkeyRange)
setup_range_and_proxy_methods(ScriptHashAddressRange)
setup_range_and_proxy_methods(WitnessScriptHashAddressRange)
setup_range_and_proxy_methods(MultisigAddressRange)
setup_range_and_proxy_methods(NonstandardAddressRange)
setup_range_and_proxy_methods(OpReturnRange)

setup_range_and_proxy_methods(cluster.ClusterRange)
setup_range_and_proxy_methods(cluster.TaggedClusterRange)
setup_range_and_proxy_methods(cluster.TaggedAddressRange)


def txes_including_output_of_type(txes, typ):
    return txes.where(lambda tx: tx.outputs.any(lambda o: o.address_type == typ))

TxIterator.including_output_of_type = txes_including_output_of_type

TxRange.including_output_of_type = txes_including_output_of_type


def inputs_sent_before_height(inputs, height: int) -> InputIterator:
    """Filter the inputs to include only inputs which spent an output created before the given height
    """
    return inputs.where(lambda inp: inp.spent_tx.block.height < height)


def inputs_sent_after_height(inputs, height: int) -> InputIterator:
    """Filter the inputs to include only inputs which spent an output created after the given height
    """
    return inputs.where(lambda inp: inp.spent_tx.block.height >= height)


def inputs_with_age_less_than(inputs, age: int) -> InputIterator:
    """Filter the inputs to include only inputs with age less than the given value
    """
    return inputs.where(lambda inp: inp.tx.block_height - inp.spent_tx.block.height < age)


def inputs_with_age_greater_than(inputs, age: int) -> InputIterator:
    """Filter the inputs to include only inputs with age more than the given value
    """
    return inputs.where(lambda inp: inp.tx.block_height - inp.spent_tx.block.height >= age)


def inputs_with_address_type(inputs, typ: address_type) -> InputIterator:
    """Filter the inputs to include only inputs that came from an address with the given type
    """
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

def outputs_spent_after_height(outputs, height):
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


def coinjoin_txes(txes):
    return txes.where(heuristics.is_coinjoin)

def possible_coinjoin_txes(txes):
    return txes.where(heuristics.is_possible_coinjoin)

def address_deanon_txes(txes):
    return txes.where(heuristics.is_address_deanon)

def change_over_txes(txes):
    return txes.where(heuristics.is_change_over)

def keyset_change_txes(txes):
    return txes.where(heuristics.is_keyset_change)

old_power_of_ten_value = heuristics.change.power_of_ten_value
def new_power_of_ten_value(digits, tx=None):
    if tx is None:
        return old_power_of_ten_value(digits)
    else:
        return old_power_of_ten_value(digits)(tx)

heuristics.change.power_of_ten_value = new_power_of_ten_value
heuristics.coinjoin_txes = coinjoin_txes
heuristics.possible_coinjoin_txes = possible_coinjoin_txes
heuristics.address_deanon_txes = address_deanon_txes
heuristics.change_over_txes = change_over_txes
heuristics.keyset_change_txes = keyset_change_txes


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
