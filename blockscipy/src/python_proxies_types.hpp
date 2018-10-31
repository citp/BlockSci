//
//  python_proxies_types.hpp
//  blocksci
//
//  Created by Harry Kalodner on 10/31/18.
//

#ifndef python_proxies_types_h
#define python_proxies_types_h

#include "proxy_py.hpp"

struct MainProxies {
    AllProxyClasses<blocksci::Block> block;
    AllProxyClasses<blocksci::Transaction> tx;
    AllProxyClasses<blocksci::Input> input;
    AllProxyClasses<blocksci::Output> output;
    AllProxyClasses<blocksci::EquivAddress> equivAddress;

    AllProxyClasses<blocksci::Cluster> cluster;
    AllProxyClasses<blocksci::TaggedCluster> taggedCluster;
    AllProxyClasses<blocksci::TaggedAddress> taggedAddress;

    MainProxies(pybind11::module &m, pybind11::class_<ProxyIterator> &proxyIteratorCl, pybind11::class_<ProxyRange> &proxyRangeCl);
};

struct ScriptProxies {
    pybind11::class_<ProxyAddress> genericAddress;
    AllProxyClasses<blocksci::AnyScript> address;
    AllProxyClasses<blocksci::script::Pubkey> pubkey;
    AllProxyClasses<blocksci::script::PubkeyHash> pubkeyHash;
    AllProxyClasses<blocksci::script::WitnessPubkeyHash> witnessPubkeyHash;
    AllProxyClasses<blocksci::script::MultisigPubkey> multisigPubkey;
    AllProxyClasses<blocksci::script::Multisig> multisig;
    AllProxyClasses<blocksci::script::ScriptHash> scripthash;
    AllProxyClasses<blocksci::script::WitnessScriptHash> witnessScripthash;
    AllProxyClasses<blocksci::script::OpReturn> nulldata;
    AllProxyClasses<blocksci::script::Nonstandard> nonstandard;
    AllProxyClasses<blocksci::script::WitnessUnknown> witnessUnknown;

    ScriptProxies(pybind11::module &m, pybind11::class_<ProxyIterator> &proxyIteratorCl, pybind11::class_<ProxyRange> &proxyRangeCl);
};

struct OtherProxies {
    AllProxyClasses<blocksci::AddressType::Enum> addressType;
    AllProxyClasses<int64_t> integer;
    AllProxyClasses<bool> boolean;
    AllProxyClasses<std::chrono::system_clock::time_point> time;
    AllProxyClasses<blocksci::uint256> uint256;
    AllProxyClasses<blocksci::uint160> uint160;
    AllProxyClasses<std::string> string;
    AllProxyClasses<pybind11::bytes> bytes;
    AllProxyClasses<pybind11::list> list;

    OtherProxies(pybind11::module &m, pybind11::class_<ProxyIterator> &proxyIteratorCl, pybind11::class_<ProxyRange> &proxyRangeCl);
};

#endif /*python_proxies_types_h*/
