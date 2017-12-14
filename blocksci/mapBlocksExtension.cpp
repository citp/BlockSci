#include <blocksci/blocksci.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

using namespace blocksci;
namespace py = pybind11;

auto mapFunc(const Block &block) {
    return ${func_def};
}


PYBIND11_MODULE(${module_name}, m) {
    m.def("func", [](const Blockchain &chain, uint32_t start, uint32_t stop) {
    	return chain.map<decltype(mapFunc(std::declval<Block>()))>(start, stop, mapFunc);
    });
}