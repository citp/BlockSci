#include <blocksci/blocksci.hpp>
#include <blocksci/hash_index.hpp>


//#include <hsql/SQLParser.h>



#include <boost/optional/optional_io.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/range/counting_range.hpp>

#include <numeric>
#include <chrono>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <string>
using namespace blocksci;

int main() {
    
    Blockchain chain("../../../../zcash-devdata");
    auto blk = chain[397];
    auto tx = blk[1];
    //std::vector<uint64_t> vpub = tx.vpubold();
    //std::cout<<tx.getTxIndex();
    Transaction::txWithIndex(398);
}
