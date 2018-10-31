//
//  python_proxies_main.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "python_proxies.hpp"
#include "python_proxies_types.hpp"
#include "caster_py.hpp"

#include "scripts/address_proxy_py.hpp"
#include "scripts/pubkey/pubkey/pubkey_proxy_py.hpp"
#include "scripts/pubkey/pubkeyhash/pubkeyhash_proxy_py.hpp"
#include "scripts/pubkey/witness_pubkeyhash/witness_pubkeyhash_proxy_py.hpp"
#include "scripts/pubkey/multisig_pubkey/multisig_pubkey_proxy_py.hpp"
#include "scripts/multisig/multisig_proxy_py.hpp"
#include "scripts/scripthash/scripthash/scripthash_proxy_py.hpp"
#include "scripts/scripthash/witness_scripthash/witness_scripthash_proxy_py.hpp"
#include "scripts/nulldata/nulldata_proxy_py.hpp"
#include "scripts/nonstandard/nonstandard_proxy_py.hpp"
#include "scripts/witness_unknown/witness_unknown_proxy_py.hpp"

namespace py = pybind11;
using namespace blocksci;

ScriptProxies::ScriptProxies(py::module &m, py::class_<ProxyIterator> &proxyIteratorCl, py::class_<ProxyRange> &proxyRangeCl) :
genericAddress(m, "ProxyAddress"),
address(m, genericAddress, proxyIteratorCl, proxyRangeCl),
pubkey(m, genericAddress, proxyIteratorCl, proxyRangeCl),
pubkeyHash(m, genericAddress, proxyIteratorCl, proxyRangeCl),
witnessPubkeyHash(m, genericAddress, proxyIteratorCl, proxyRangeCl),
multisigPubkey(m, genericAddress, proxyIteratorCl, proxyRangeCl),
multisig(m, genericAddress, proxyIteratorCl, proxyRangeCl),
scripthash(m, genericAddress, proxyIteratorCl, proxyRangeCl),
witnessScripthash(m, genericAddress, proxyIteratorCl, proxyRangeCl),
nulldata(m, genericAddress, proxyIteratorCl, proxyRangeCl),
nonstandard(m, genericAddress, proxyIteratorCl, proxyRangeCl),
witnessUnknown(m, genericAddress, proxyIteratorCl, proxyRangeCl) {}

void setupScriptProxies(ScriptProxies &proxies) {
    init_proxy_address(proxies.genericAddress);
    addAddressProxyMethods(proxies.address);
    addPubkeyProxyMethods(proxies.pubkey);
    addPubkeyHashProxyMethods(proxies.pubkeyHash);
    addWitnessPubkeyHashProxyMethods(proxies.witnessPubkeyHash);
    addMultisigPubkeyProxyMethods(proxies.multisigPubkey);
    addMultisigProxyMethods(proxies.multisig);
    addScriptHashProxyMethods(proxies.scripthash);
    addWitnessScriptHashProxyMethods(proxies.witnessScripthash);
    addNonstandardProxyMethods(proxies.nonstandard);
    addNulldataProxyMethods(proxies.nulldata);
    addWitnessUnknownProxyMethods(proxies.witnessUnknown);
}