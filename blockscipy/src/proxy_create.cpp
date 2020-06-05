//
//  proxy_create.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/15/18.
//

#include "proxy_create.hpp"

#include <boost/core/demangle.hpp>

#include <sstream>

using namespace blocksci;
using namespace blocksci::script;

Proxy<AnyScript> SimpleProxyCreator<AnyScript>::operator()() const {
	return {std::function<AnyScript(std::any &)>{[](std::any &t) -> AnyScript {
		const std::type_info &o = t.type();
		if (o == typeid(Pubkey)) {
			return std::any_cast<Pubkey>(t);
		} else if (o == typeid(PubkeyHash)) {
			return std::any_cast<PubkeyHash>(t);
		} else if (o == typeid(WitnessPubkeyHash)) {
			return std::any_cast<WitnessPubkeyHash>(t);
		} else if (o == typeid(MultisigPubkey)) {
			return std::any_cast<MultisigPubkey>(t);
		} else if (o == typeid(Multisig)) {
			return std::any_cast<Multisig>(t);
		} else if (o == typeid(ScriptHash)) {
			return std::any_cast<ScriptHash>(t);
		} else if (o == typeid(WitnessScriptHash)) {
			return std::any_cast<WitnessScriptHash>(t);
		} else if (o == typeid(OpReturn)) {
			return std::any_cast<OpReturn>(t);
		} else if (o == typeid(Nonstandard)) {
			return std::any_cast<Nonstandard>(t);
		} else if (o == typeid(WitnessUnknown)) {
			return std::any_cast<WitnessUnknown>(t);
		} else if (o == typeid(AnyScript)) {
			return std::any_cast<AnyScript>(t);
		} else {
			std::stringstream ss;
			ss << "Proxy type error: " << boost::core::demangle(o.name()) << " does not match AnyScript";
			throw std::runtime_error(ss.str());
		}
	}}, createProxyTypeInfo<AnyScript>()};
}