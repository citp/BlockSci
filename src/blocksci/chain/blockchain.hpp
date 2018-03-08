//
//  blockchain.hpp
//  BlockReader
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef blockchain_hpp
#define blockchain_hpp

#include "block.hpp"
#include "transaction.hpp"
#include <blocksci/scripts/script_access.hpp>
#include <blocksci/scripts/script_variant.hpp>
#include <blocksci/util/data_access.hpp>

#include <range/v3/view_facade.hpp>
#include <range/v3/view/any_view.hpp>
#include <range/v3/range_for.hpp>

#include <type_traits>
#include <future>

namespace blocksci {
    struct DataConfiguration;
    class DataAccess;
    
    template <typename F, typename... Args>
    struct is_callable {
        template <typename U>
        static auto test(U* p) -> decltype((*p)(std::declval<Args>()...), void(), std::true_type());
        
        template <typename U>
        static auto test(...) -> decltype(std::false_type());
        
        static constexpr bool value = decltype(test<F>(nullptr))::value;
    };
    
    template <typename ResultType, typename It, typename MapFunc, typename ReduceFunc>
    ResultType mapReduceBlocksImp(It begin, It end, MapFunc mapFunc, ReduceFunc reduceFunc) {
        auto segmentCount = std::distance(begin, end);
        if(segmentCount == 1) {
            ResultType res{};
            auto ret = mapFunc(*begin);
            res = reduceFunc(res, ret);
            return res;
        } else {
            auto mid = begin;
            std::advance(mid, segmentCount / 2);
            auto handle = std::async(std::launch::async, mapReduceBlocksImp<ResultType, It, MapFunc, ReduceFunc>, begin, mid, mapFunc, reduceFunc);
            ResultType res{};
            auto ret1 = mapReduceBlocksImp<ResultType>(mid, end, mapFunc, reduceFunc);
            res = reduceFunc(res, ret1);
            auto ret2 = handle.get();
            res = reduceFunc(res, ret2);
            return res;
        }
    }
    
    class Blockchain;
    
    std::vector<std::vector<Block>> segmentChain(const Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, unsigned int segmentCount);
    
    template<typename T>
    std::vector<std::vector<Block>> segmentBlocks(T && chain, int segmentCount) {
        auto lastTx = chain.back().endTxIndex();
        auto firstTx = chain.front().firstTxIndex();
        uint32_t segmentSize = (lastTx - firstTx) / segmentCount;
        
        std::vector<std::vector<Block>> segments;
        auto it = chain.begin();
        auto chainEnd = chain.end();
        uint32_t startIndex = 0;

        auto blockIndexCmp = [](const Block &block, uint32_t txNum) {
            return block.firstTxIndex() < txNum;
        };

        for (int i = 0; i < segmentCount - 1; i++) {
            auto endIt = std::lower_bound(it, chainEnd, startIndex + segmentSize, blockIndexCmp);
            startIndex = endIt->firstTxIndex();
            segments.push_back(std::vector<Block>(it, endIt));
            it = endIt;
        }
        segments.push_back(std::vector<Block>(it, chainEnd));
        return segments;
    }
    
    uint32_t txCount(const Blockchain &chain);
    
    template <AddressType::Enum type>
    using ScriptRange = ranges::any_view<ScriptAddress<type>>;
    using ScriptRangeVariant = to_variadic_t<to_address_tuple_t<ScriptRange>, mpark::variant>;
    
    class Blockchain : public ranges::view_facade<Blockchain> {
        friend ranges::range_access;
        
        struct cursor {
        private:
            const Blockchain *chain;
            BlockHeight currentBlockHeight;
        public:
            cursor() = default;
            cursor(const Blockchain &chain_, BlockHeight height) : chain(&chain_), currentBlockHeight(height) {}
            
            bool equal(const cursor &other) const {
                return currentBlockHeight == other.currentBlockHeight;
            }
            
            Block read() const {
                return Block(currentBlockHeight, *chain->access->chain);
            }
            
            void next() {
                currentBlockHeight++;
            }
            
            void prev() {
                currentBlockHeight--;
            }
            
            int distance_to(cursor const &that) const {
                return static_cast<int>(that.currentBlockHeight - currentBlockHeight);
            }
            
            void advance(int amount) {
                currentBlockHeight += BlockHeight{amount};
            }
            
            bool equal(ranges::default_sentinel) const {
                return currentBlockHeight == chain->lastBlockHeight;
            }
            
            BlockHeight distance_to(ranges::default_sentinel) const {
                return chain->lastBlockHeight - currentBlockHeight;
            }
        };
        
        cursor begin_cursor() const {
            return cursor(*this, BlockHeight{0});
        }
        
        ranges::default_sentinel end_cursor() const {
            return {};
        }
        
        BlockHeight lastBlockHeight;

    public:
        Blockchain() = default;
        Blockchain(const DataConfiguration &config, bool errorOnReorg, BlockHeight blocksIgnored);
        Blockchain(const std::string &dataDirectory);
        
        const DataAccess *access;
        
        uint32_t firstTxIndex() const;
        uint32_t endTxIndex() const;
        
        BlockHeight size() const {
            return lastBlockHeight;
        }
        
        template <AddressType::Enum type>
        auto scripts() const {
            return ranges::view::iota(uint32_t{1}, access->scripts->scriptCount<equivType(type)>() + 1) | ranges::view::transform([&](uint32_t scriptNum) {
                assert(scriptNum > 0);
                return ScriptAddress<type>(*access->scripts, scriptNum);
            });
        }
        
        ScriptRangeVariant scripts(AddressType::Enum type) const;
        
        template <typename ResultType, typename MapFunc, typename ReduceFunc>
        std::enable_if_t<is_callable<MapFunc, std::vector<Block>>::value, ResultType>
        mapReduce(BlockHeight start, BlockHeight stop, MapFunc mapFunc, ReduceFunc reduceFunc) const {
            auto segments = segmentChain(*this, start, stop, std::thread::hardware_concurrency());
            return mapReduceBlocksImp<ResultType>(segments.begin(), segments.end(), mapFunc, reduceFunc);
        }

        template <typename ResultType, typename MapFunc, typename ReduceFunc>
        std::enable_if_t<is_callable<MapFunc, Block>::value, ResultType>
        mapReduce(BlockHeight start, BlockHeight stop, MapFunc mapFunc, ReduceFunc reduceFunc) const {
            auto mapF = [&](const std::vector<Block> &segment) {
                ResultType res{};
                for (auto &block : segment) {
                    auto mapped = mapFunc(block);
                    res = reduceFunc(res, mapped);
                }
                return res;
            };
            return mapReduce<ResultType>(start, stop, mapF, reduceFunc);
        }

        template <typename ResultType, typename MapFunc, typename ReduceFunc>
        std::enable_if_t<is_callable<MapFunc, Transaction>::value, ResultType>
        mapReduce(BlockHeight start, BlockHeight stop, MapFunc mapFunc, ReduceFunc reduceFunc) const {
            auto mapF = [&](const Block &block) {
                ResultType res{};
                RANGES_FOR(auto tx, block) {
                    auto mapped = mapFunc(tx);
                    res = reduceFunc(res, mapped);
                }
                return res;
            };
            
            return mapReduce<ResultType>(start, stop, mapF, reduceFunc);
        }
        
        template <typename MapType>
        std::vector<MapType> map(BlockHeight start, BlockHeight stop, const std::function<MapType(const Block &)> &mapFunc) const {
            auto mapF = [&](const std::vector<Block> &segment) {
                std::vector<MapType> vec;
                vec.reserve(segment.size());
                for (auto &block : segment) {
                    vec.push_back(mapFunc(block));
                }
                return vec;
            };
            
            auto reduceFunc = [](std::vector<MapType> &vec1, std::vector<MapType> &vec2) -> std::vector<MapType> & {
                vec1.reserve(vec1.size() + vec2.size());
                vec1.insert(vec1.end(), std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));
                return vec1;
            };
            
            return mapReduce<std::vector<MapType>>(start, stop, mapF, reduceFunc);
        }
    };
    
    CONCEPT_ASSERT(ranges::Range<Blockchain>());
    CONCEPT_ASSERT(ranges::InputRange<Blockchain>());
    CONCEPT_ASSERT(ranges::ForwardRange<Blockchain>());
    CONCEPT_ASSERT(ranges::BidirectionalRange<Blockchain>());
    CONCEPT_ASSERT(ranges::RandomAccessRange<Blockchain>());
    
    // filter - Blocks and Txes
    std::vector<Block> filter(const Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, std::function<bool(const Block &block)> testFunc);
    std::vector<Transaction> filter(const Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, std::function<bool(const Transaction &tx)> testFunc);
    
    std::vector<Transaction> getTransactionIncludingOutput(const Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, AddressType::Enum type);
}


#endif /* blockchain_hpp */
