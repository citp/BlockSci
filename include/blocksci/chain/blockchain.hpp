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

#include <blocksci/blocksci_export.h>
#include <blocksci/scripts/multisig_script.hpp>
#include <blocksci/scripts/nonstandard_script.hpp>
#include <blocksci/scripts/nulldata_script.hpp>
#include <blocksci/scripts/pubkey_script.hpp>
#include <blocksci/scripts/multisig_pubkey_script.hpp>
#include <blocksci/scripts/scripthash_script.hpp>
#include <blocksci/util/data_access.hpp>

#include <range/v3/view/any_view.hpp>
#include <range/v3/range_for.hpp>

#include <mpark/variant.hpp>

#include <map>
#include <type_traits>
#include <future>

namespace blocksci {
    struct DataConfiguration;
    class DataAccess;
    
    template <AddressType::Enum type>
    using ScriptRange = ranges::any_view<ScriptAddress<type>, ranges::category::random_access>;
    using ScriptRangeVariant = to_variadic_t<to_address_tuple_t<ScriptRange>, mpark::variant>;
    
    namespace internal {
        template<AddressType::Enum type>
        struct ScriptRangeFunctor {
            static ScriptRangeVariant f(Blockchain &chain);
        };
        
        template <typename F, typename... Args>
        struct is_callable {
            template <typename U>
            static auto test(U* p) -> decltype((*p)(std::declval<Args>()...), void(), std::true_type());
            
            template <typename U>
            static auto test(...) -> decltype(std::false_type());
            
            static constexpr bool value = decltype(test<F>(nullptr))::value;
        };
        
        template <typename ResultType, typename It, typename MapFunc, typename ReduceFunc>
        ResultType mapReduceBlocksImp(It begin, It end, MapFunc mapFunc, ReduceFunc reduceFunc, int segmentBeginNum) {
            auto segmentCount = static_cast<int>(std::distance(begin, end));
            if(segmentCount == 1) {
                ResultType res{};
                auto ret = mapFunc(*begin, segmentBeginNum);
                res = reduceFunc(res, ret);
                return res;
            } else {
                auto mid = begin;
                std::advance(mid, segmentCount / 2);
                auto handle = std::async(std::launch::async, mapReduceBlocksImp<ResultType, It, MapFunc, ReduceFunc>, begin, mid, mapFunc, reduceFunc, segmentBeginNum);
                ResultType res{};
                auto ret2 = mapReduceBlocksImp<ResultType>(mid, end, mapFunc, reduceFunc, segmentBeginNum + segmentCount / 2);
                auto ret1 = handle.get();
                res = reduceFunc(res, ret1);
                res = reduceFunc(res, ret2);
                return res;
            }
        }
    }
    
    std::vector<std::vector<Block>> BLOCKSCI_EXPORT segmentChain(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, unsigned int segmentCount);
    std::vector<std::pair<int, int>> BLOCKSCI_EXPORT segmentChainIndexes(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, unsigned int segmentCount);
    
    template<typename T>
    std::vector<std::vector<Block>> BLOCKSCI_EXPORT segmentBlocks(T && chain, int segmentCount) {
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
    
    class BLOCKSCI_EXPORT Blockchain { 
        BlockHeight lastBlockHeight;

        DataAccess access;
        
    public:
        
        class iterator {
        public:
            using self_type = iterator;
            using value_type = Block;
            using pointer = Block;
            using reference = Block;
            using difference_type = BlockHeight;
            using iterator_category = std::random_access_iterator_tag;

            iterator() = default;
            iterator(BlockHeight height_, DataAccess *access_) : height(height_), access(access_) { }
            self_type &operator+=(difference_type i) { height += i; return *this; }
            self_type &operator-=(difference_type i) { height -= i; return *this; }
            self_type &operator++() { ++height; return *this; }
            self_type &operator--() { --height; return *this; }
            self_type operator++(int) { self_type tmp = *this; ++height; return tmp; }
            self_type operator--(int) { self_type tmp = *this; --height; return tmp; }
            self_type operator+(difference_type i) { self_type tmp = *this; tmp.height += i; return tmp; }
            self_type operator-(difference_type i) { self_type tmp = *this; tmp.height -= i; return tmp; }

            value_type operator*() { return Block(height, *access); }
            bool operator==(const self_type& rhs) { return height == rhs.height; }
            bool operator!=(const self_type& rhs) { return height != rhs.height; }
            BlockHeight operator-(const self_type& it){return height - it.height;}
        private:
            BlockHeight height;
            DataAccess *access;
        };
        
        Blockchain() = default;
        explicit Blockchain(const DataConfiguration &config) : access(config) {
            lastBlockHeight = access.getChain().blockCount();
        }
        explicit Blockchain(const std::string &dataDirectory) : Blockchain(DataConfiguration{dataDirectory, true, BlockHeight{0}}) {}
        
        auto blocks() {
            auto dataAccess = &getAccess();
            return ranges::view::ints(BlockHeight{0}, lastBlockHeight) |
                   ranges::view::transform([dataAccess](BlockHeight height) {return Block(height, *dataAccess); });
        }

        void reload() {
            access.reload();
            lastBlockHeight = access.getChain().blockCount();
        }

        auto begin() {
            return iterator(0, &getAccess());
        }

        auto end() {
            return iterator(lastBlockHeight, &getAccess());
        }

        auto operator[](BlockHeight height) {
            return Block(height, getAccess());
        }
        
        DataAccess &getAccess() { return access; }
        
        uint32_t firstTxIndex() const;
        uint32_t endTxIndex() const;
        
        uint32_t size() const {
            return static_cast<uint32_t>(lastBlockHeight);
        }
        
        uint32_t addressCount(AddressType::Enum type) const {
            return access.getScripts().scriptCount(dedupType(type));
        }
        
        template <AddressType::Enum type>
        auto scripts() {
            return ranges::view::ints(uint32_t{1}, access.getScripts().scriptCount(dedupType(type)) + 1) | ranges::view::transform([&](uint32_t scriptNum) {
                return ScriptAddress<type>(scriptNum, access);
            });
        }
        
        ScriptRangeVariant scripts(AddressType::Enum type) {
            static constexpr auto table = make_dynamic_table<AddressType, internal::ScriptRangeFunctor>();
            auto index = static_cast<size_t>(type);
            return table.at(index)(*this);
        }
        
        template <typename ResultType, typename MapFunc, typename ReduceFunc>
        std::enable_if_t<internal::is_callable<MapFunc, std::vector<Block>, int>::value, ResultType>
        mapReduce(BlockHeight start, BlockHeight stop, MapFunc mapFunc, ReduceFunc reduceFunc) {
            auto segments = segmentChain(*this, start, stop, std::thread::hardware_concurrency());
            return internal::mapReduceBlocksImp<ResultType>(segments.begin(), segments.end(), mapFunc, reduceFunc, 0);
        }
        
        template <typename ResultType, typename MapFunc, typename ReduceFunc>
        std::enable_if_t<internal::is_callable<MapFunc, std::vector<Block>>::value, ResultType>
        mapReduce(BlockHeight start, BlockHeight stop, MapFunc mapFunc, ReduceFunc reduceFunc) {
            auto segments = segmentChain(*this, start, stop, std::thread::hardware_concurrency());
            return internal::mapReduceBlocksImp<ResultType>(segments.begin(), segments.end(), [&](const std::vector<Block> &blocks, int) { return mapFunc(blocks); }, reduceFunc, 0);
        }

        template <typename ResultType, typename MapFunc, typename ReduceFunc>
        std::enable_if_t<internal::is_callable<MapFunc, Block>::value, ResultType>
        mapReduce(BlockHeight start, BlockHeight stop, MapFunc mapFunc, ReduceFunc reduceFunc) {
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
        std::enable_if_t<internal::is_callable<MapFunc, Transaction>::value, ResultType>
        mapReduce(BlockHeight start, BlockHeight stop, MapFunc mapFunc, ReduceFunc reduceFunc) {
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
        std::vector<MapType> map(BlockHeight start, BlockHeight stop, const std::function<MapType(const Block &)> &mapFunc) {
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
    
    uint32_t BLOCKSCI_EXPORT txCount(Blockchain &chain);
    
    // filter - Blocks and Txes
    std::vector<Block> BLOCKSCI_EXPORT filter(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, std::function<bool(const Block &block)> testFunc);
    std::vector<Transaction> BLOCKSCI_EXPORT filter(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, std::function<bool(const Transaction &tx)> testFunc);
    
    std::vector<Transaction> BLOCKSCI_EXPORT getTransactionIncludingOutput(Blockchain &chain, BlockHeight startBlock, BlockHeight endBlock, AddressType::Enum type);
    
    std::map<int64_t, Address> BLOCKSCI_EXPORT mostValuableAddresses(Blockchain &chain);
    
    namespace internal {
        template<AddressType::Enum type>
        ScriptRangeVariant ScriptRangeFunctor<type>::f(Blockchain &chain) {
            return chain.scripts<type>();
        }
    }
} // namespace blocksci


#endif /* blockchain_hpp */
