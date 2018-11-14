//
//  proxy_type_check.cpp
//  blocksci
//
//  Created by Harry Kalodner on 11/14/18.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#include "proxy_type_check.hpp"

#include <boost/core/demangle.hpp>

#include <stdexcept>
#include <sstream>

void proxyTypeCheck(const std::type_info *s1, const std::type_info *s2) {
	if (s1 != nullptr && s2 != nullptr && *s1 != *s2) {
		std::stringstream ss;
		ss << "Proxy type error: " << boost::core::demangle(s2->name()) << " does not match " << boost::core::demangle(s1->name());
		throw std::runtime_error(ss.str());
	}
}