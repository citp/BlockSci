//
//  proxy_functions.hpp
//  blocksci
//
//  Created by Harry Kalodner on 11/18/18.
//

#ifndef proxy_functions_impl_hpp
#define proxy_functions_impl_hpp

void addProxyFunctionsCore(pybind11::module &m, pybind11::module &pm);
void addProxyFunctionsScripts(pybind11::module &m, pybind11::module &pm);
void addProxyFunctionsOther(pybind11::module &m, pybind11::module &pm);

void addProxyFlowFunctionsCore(pybind11::module &m, pybind11::module &pm);
void addProxyFlowFunctionsScripts(pybind11::module &m, pybind11::module &pm);
void addProxyFlowFunctionsOther(pybind11::module &m, pybind11::module &pm);

inline void defineProxyFunctions(pybind11::module &m, pybind11::module &pm) {
    addProxyFunctionsCore(m, pm);
    addProxyFunctionsScripts(m, pm);
    addProxyFunctionsOther(m, pm);

    addProxyFlowFunctionsCore(m, pm);
    addProxyFlowFunctionsScripts(m, pm);
    addProxyFlowFunctionsOther(m, pm);
}

#endif /* proxy_functions_impl_hpp */
