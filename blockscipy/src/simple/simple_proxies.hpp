//
//  simple_proxies.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef simple_proxies_h
#define simple_proxies_h

#include "python_fwd.hpp"

#include <blocksci/blocksci_fwd.hpp>

#include <pybind11/pybind11.h>

#include <chrono>

void addIntProxyMethods(AllProxyClasses<int64_t> &cls);
void addBoolProxyMethods(AllProxyClasses<bool> &cls);
void addAddressTypeProxyMethods(AllProxyClasses<blocksci::AddressType::Enum> &cls);

void addTimeProxyMethods(AllProxyClasses<std::chrono::system_clock::time_point> &cls);
void addUint256ProxyMethods(AllProxyClasses<blocksci::uint256> &cls);
void addUint160ProxyMethods(AllProxyClasses<blocksci::uint160> &cls);
void addBytesProxyMethods(AllProxyClasses<pybind11::bytes> &cls);
void addStringProxyMethods(AllProxyClasses<std::string> &cls);
void addListProxyMethods(AllProxyClasses<pybind11::list> &cls);
#endif /* simple_proxies_h */
