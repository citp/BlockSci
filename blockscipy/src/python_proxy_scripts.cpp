//
//  python_proxies_main.cpp
//  blocksci_interface
//
//  Created by Harry Kalodner on 4/21/18.
//

#include "python_proxies.hpp"
#include "python_proxies_types.hpp"
#include "caster_py.hpp"
#include "proxy_py_create.hpp"

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

#include <blocksci/chain/block.hpp>

namespace py = pybind11;
using namespace blocksci;

ScriptProxies::ScriptProxies(py::module &m) :
genericAddress(m, "ProxyAddress"),
address(createProxyClasses<AnyScript, ProxyAddress>(m)),
pubkey(createProxyClasses<script::Pubkey, ProxyAddress>(m)),
pubkeyHash(createProxyClasses<script::PubkeyHash, ProxyAddress>(m)),
witnessPubkeyHash(createProxyClasses<script::WitnessPubkeyHash, ProxyAddress>(m)),
multisigPubkey(createProxyClasses<script::MultisigPubkey, ProxyAddress>(m)),
multisig(createProxyClasses<script::Multisig, ProxyAddress>(m)),
scripthash(createProxyClasses<script::ScriptHash, ProxyAddress>(m)),
witnessScripthash(createProxyClasses<script::WitnessScriptHash, ProxyAddress>(m)),
nulldata(createProxyClasses<script::OpReturn, ProxyAddress>(m)),
nonstandard(createProxyClasses<script::Nonstandard, ProxyAddress>(m)),
witnessUnknown(createProxyClasses<script::WitnessUnknown, ProxyAddress>(m)) {}

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