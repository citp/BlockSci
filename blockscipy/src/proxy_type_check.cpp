//
//  proxy_type_check.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/14/18.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "proxy_type_check.hpp"

#include <boost/core/demangle.hpp>

#include <blocksci/scripts/script_variant.hpp>

#include <stdexcept>
#include <sstream>

using namespace blocksci;

namespace {
	bool kindAccept(ProxyType a, ProxyType b) {
		return
			(a == b) ||
			(a == ProxyType::Optional && b == ProxyType::Simple) ||
			(a == ProxyType::Iterator && b == ProxyType::Range);
	}

	bool isScriptType(const std::type_info &type) {
		return
			type == typeid(script::Pubkey) ||
			type == typeid(script::PubkeyHash) ||
			type == typeid(script::WitnessPubkeyHash) ||
			type == typeid(script::MultisigPubkey) ||
			type == typeid(script::Multisig) ||
			type == typeid(script::ScriptHash) ||
			type == typeid(script::WitnessScriptHash) ||
			type == typeid(script::OpReturn) ||
			type == typeid(script::Nonstandard) ||
			type == typeid(script::WitnessUnknown);
	}
}

void ProxyTypeInfo::checkAccept(const ProxyTypeInfo &other) const {
	if (kindAccept(kind, other.kind)) {
		if (baseType == nullptr || other.baseType == nullptr || *baseType == *other.baseType) {
			return;
		}

		if (*baseType == typeid(AnyScript) && isScriptType(*other.baseType)) {
			return;
		}
	}

	std::stringstream ss;
	ss << "Proxy type error: " << boost::core::demangle(type->name()) << " does not accept " << boost::core::demangle(other.type->name());
	throw std::runtime_error(ss.str());
}

void ProxyTypeInfo::checkMatch(const ProxyTypeInfo &other) const {
	if (type == nullptr || other.type == nullptr || *type == *other.type) {
		return;
	}

	std::stringstream ss;
	ss << "Proxy type error: " << boost::core::demangle(other.type->name()) << " does not match " << boost::core::demangle(type->name());
	throw std::runtime_error(ss.str());
}
