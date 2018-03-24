//
//  export.cpp
//  blocksci
//
//  Created by Malte Möser on 12/3/17.
//

#include <blocksci/chain/blockchain.hpp>
#include <blocksci/heuristics/change_address.hpp>
#include <blocksci/heuristics/tx_identification.hpp>

#include <bitset>
#include <vector>
#include <fstream>
#include <iostream>

namespace blocksci {
    
    uint32_t compactAddress(const Address &address) {
        return address.scriptNum | (static_cast<uint8_t>(address.type) << 29);
    }

    void printToFile(const std::unordered_set<Output> &outputs, std::ofstream &file) {
        uint16_t addressCount = outputs.size();
        file.write(reinterpret_cast<const char *>(&addressCount), sizeof(addressCount));
        for (auto &output : outputs) {
            auto compact = compactAddress(output.getAddress());
            file.write(reinterpret_cast<const char *>(&compact), sizeof(compact));
        }
    }
    
    // iterate over every tx in the chain
    void parseChain(const Blockchain &chain, std::ofstream &file) {

        RANGES_FOR(auto block, chain) {
            uint32_t height = block.height();
            RANGES_FOR(auto tx, block) {
                if(tx.isCoinbase() || heuristics::isCoinjoin(tx)) {
                    continue;
                }
                
                auto lockTime = heuristics::changeByLocktime(tx);
                if (lockTime.size() == tx.inputs().size()) {
                    lockTime.clear();
                }

                auto scriptType = heuristics::changeByScriptType(tx);
                if (scriptType.size() == tx.inputs().size()) {
                    scriptType.clear();
                }

                auto reuse = heuristics::changeByAddressReuse(tx);
                if (reuse.size() == tx.inputs().size()) {
                    reuse.clear();
                }
                
                auto peeling = heuristics::changeByPeelingChain(tx);
                if (peeling.size() == tx.inputs().size()) {
                    peeling.clear();
                }

                auto optimal = heuristics::changeByOptimalChange(tx);
                if (optimal.size() == tx.inputs().size()) {
                    optimal.clear();
                }

                auto powerOfTen = heuristics::changeByPowerOfTenValue(tx);
                if (powerOfTen.size() == tx.inputs().size()) {
                    powerOfTen.clear();
                }

                auto behavior = heuristics::changeByClientChangeAddressBehavior(tx);
                if (behavior.size() == tx.inputs().size()) {
                    behavior.clear();
                }

                auto inputs = tx.inputs();
                std::bitset<8> inputFlags;
                inputFlags[7] = 1;
                auto inputFlagSet = static_cast<uint8_t>(inputFlags.to_ulong());
                for(uint32_t i = 0; i < inputs.size() - 1; ++i) {
                    auto compact1 = compactAddress(inputs[i].getAddress());
                    for(uint32_t j = i+1; j < inputs.size(); ++j) {
                        
                        auto compact2 = compactAddress(inputs[j].getAddress());
                        file.write(reinterpret_cast<const char *>(&compact1), sizeof(compact1));
                        file.write(reinterpret_cast<const char *>(&compact2), sizeof(compact2));
                        file.write(reinterpret_cast<const char *>(&inputFlagSet), sizeof(inputFlagSet));
                        std::cout << std::bitset<32>(compact1) << " " << std::bitset<32>(compact2) << " " << std::bitset<8>(inputFlagSet) << std::endl;
                    }
                }

                for (auto output : tx.outputs()) {
                    bool isLocktime = lockTime.find(output) != lockTime.end();
                    bool isScriptType = scriptType.find(output) != scriptType.end();
                    bool isReuse = reuse.find(output) != reuse.end();
                    bool isPeeling = peeling.find(output) != peeling.end();
                    bool isOptimal = optimal.find(output) != optimal.end();
                    bool isPowerOfTen = powerOfTen.find(output) != powerOfTen.end();
                    bool isBehavior = behavior.find(output) != behavior.end();
                    bool any = isLocktime | isScriptType | isReuse | isPeeling | isOptimal | isPowerOfTen | isBehavior;
                    if (any) {
                        std::bitset<8> flags;
                        flags[0] = isLocktime;
                        flags[1] = isScriptType;
                        flags[2] = isReuse;
                        flags[3] = isPeeling;
                        flags[4] = isOptimal;
                        flags[5] = isPowerOfTen;
                        flags[6] = isBehavior;
                        flags[7] = 0;
                        auto flagSet = static_cast<uint8_t>(flags.to_ulong());
                        auto compact1 = compactAddress(output.getAddress());
                        for (auto input : tx.inputs()) {
                            auto compact2 = compactAddress(input.getAddress());
                            file.write(reinterpret_cast<const char *>(&compact1), sizeof(compact1));
                            file.write(reinterpret_cast<const char *>(&compact2), sizeof(compact2));
                            file.write(reinterpret_cast<const char *>(&flagSet), sizeof(flagSet));

                            std::cout << std::bitset<32>(compact1) << " " << std::bitset<32>(compact2) << " " << std::bitset<8>(flagSet) << std::endl;
                        }
                    }
                    
                }
                
            }

            if (height > 1000) {
                return;
            }

            if (height % 10000 == 0) {
                std::cout << "At height " << height << "\n";
            }
        }
    }    
}

int main(int argc, const char * argv[]) {
    std::ofstream file;
    file.open("output.dat", std::ios::binary);
    
    blocksci::Blockchain chain("/blocksci/bitcoin/");
    blocksci::parseChain(chain, file);
    
    file.close();
    return 0;
}

