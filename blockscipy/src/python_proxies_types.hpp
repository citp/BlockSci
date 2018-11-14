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

    MainProxies(pybind11::module &m);
};

struct ScriptProxies {
    pybind11::class_<ProxyAddress, SimpleProxy> genericAddress;
    AllProxyClasses<blocksci::AnyScript, ProxyAddress> address;
    AllProxyClasses<blocksci::script::Pubkey, ProxyAddress> pubkey;
    AllProxyClasses<blocksci::script::PubkeyHash, ProxyAddress> pubkeyHash;
    AllProxyClasses<blocksci::script::WitnessPubkeyHash, ProxyAddress> witnessPubkeyHash;
    AllProxyClasses<blocksci::script::MultisigPubkey, ProxyAddress> multisigPubkey;
    AllProxyClasses<blocksci::script::Multisig, ProxyAddress> multisig;
    AllProxyClasses<blocksci::script::ScriptHash, ProxyAddress> scripthash;
    AllProxyClasses<blocksci::script::WitnessScriptHash, ProxyAddress> witnessScripthash;
    AllProxyClasses<blocksci::script::OpReturn, ProxyAddress> nulldata;
    AllProxyClasses<blocksci::script::Nonstandard, ProxyAddress> nonstandard;
    AllProxyClasses<blocksci::script::WitnessUnknown, ProxyAddress> witnessUnknown;

    ScriptProxies(pybind11::module &m);
};

struct OtherProxies {
    AllProxyClasses<blocksci::uint256> uint256;
    AllProxyClasses<blocksci::uint160> uint160;
    AllProxyClasses<blocksci::AddressType::Enum> addressType;
    AllProxyClasses<int64_t> integer;
    AllProxyClasses<bool> boolean;
    AllProxyClasses<std::chrono::system_clock::time_point> time;
    AllProxyClasses<std::string> string;
    AllProxyClasses<pybind11::bytes> bytes;
    AllProxyClasses<pybind11::list> list;

    OtherProxies(pybind11::module &m);
};

#endif /*python_proxies_types_h*/
