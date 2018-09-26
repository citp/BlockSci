//
//  other_proxy_py.hpp
//  blocksci
//
//  Created by Harry Kalodner on 3/28/18.
//

#ifndef other_proxy_py_h
#define other_proxy_py_h

#include "proxy_py.hpp"

void addIntProxyMethods(AllProxyClasses<int64_t> &cls);
void addBoolProxyMethods(AllProxyClasses<bool> &cls);
void addAddressTypeProxyMethods(AllProxyClasses<blocksci::AddressType::Enum> &cls);

void addTimeProxyMethods(AllProxyClasses<std::chrono::system_clock::time_point> &cls);
void addUint256ProxyMethods(AllProxyClasses<blocksci::uint256> &cls);
void addUint160ProxyMethods(AllProxyClasses<blocksci::uint160> &cls);
void addBytesProxyMethods(AllProxyClasses<pybind11::bytes> &cls);
#endif /* other_proxy_py_h */
