//
//  python_proxies.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/21/18.
//

#ifndef python_proxies_h
#define python_proxies_h

#include <pybind11/pybind11.h>

struct MainProxies;
struct ScriptProxies;
struct OtherProxies;

void setupMainProxies(MainProxies &proxies);
void setupScriptProxies(ScriptProxies &proxies);
void setupOtherProxies(OtherProxies &proxies);

void setupProxies(pybind11::module &m);

#endif /* python_proxies_h */