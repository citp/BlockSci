//
//  python_proxies_main.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "python_proxies.hpp"
#include "caster_py.hpp"
#include "proxy_py.hpp"

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

void setupScriptProxies(py::module &m, py::class_<ProxySequence<ranges::category::input>> &proxyIteratorCl, py::class_<ProxySequence<random_access_sized>> &proxyRangeCl) {
    py::class_<ProxyAddress> proxyAddressCl(m, "ProxyAddress");
    AllProxyClasses<AnyScript> addressProxyCls(m, proxyAddressCl, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<script::Pubkey> pubkeyProxyCls(m, proxyAddressCl, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<script::PubkeyHash> pubkeyHashProxyCls(m, proxyAddressCl, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<script::WitnessPubkeyHash> witnessPubkeyHashProxyCls(m, proxyAddressCl, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<script::MultisigPubkey> multisigPubkeyProxyCls(m, proxyAddressCl, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<script::Multisig> multisigProxyCls(m, proxyAddressCl, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<script::ScriptHash> scripthashProxyCls(m, proxyAddressCl, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<script::WitnessScriptHash> witnessScripthashProxyCls(m, proxyAddressCl, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<script::OpReturn> nulldataProxyCls(m, proxyAddressCl, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<script::Nonstandard> nonstandardProxyCls(m, proxyAddressCl, proxyIteratorCl, proxyRangeCl);
    AllProxyClasses<script::WitnessUnknown> witnessUnknownProxyCls(m, proxyAddressCl, proxyIteratorCl, proxyRangeCl);

    init_proxy_address(proxyAddressCl);
    addAddressProxyMethods(addressProxyCls);
    addPubkeyProxyMethods(pubkeyProxyCls);
    addPubkeyHashProxyMethods(pubkeyHashProxyCls);
    addWitnessPubkeyHashProxyMethods(witnessPubkeyHashProxyCls);
    addMultisigPubkeyProxyMethods(multisigPubkeyProxyCls);
    addMultisigProxyMethods(multisigProxyCls);
    addScriptHashProxyMethods(scripthashProxyCls);
    addWitnessScriptHashProxyMethods(witnessScripthashProxyCls);
    addNonstandardProxyMethods(nonstandardProxyCls);
    addNulldataProxyMethods(nulldataProxyCls);
    addWitnessUnknownProxyMethods(witnessUnknownProxyCls);
}