//
//  block_range.hpp
//  blocksci
//
//  Created by Harry Kalodner on 1/3/17.
//  Copyright © 2017 Harry Kalodner. All rights reserved.
//

#ifndef block_range_hpp
#define block_range_hpp

#include <blocksci/blocksci_export.h>
#include <blocksci/chain/block.hpp>

#include <map>
#include <type_traits>
#include <future>

namespace blocksci {
    struct DataConfiguration;
    class DataAccess;
    
    namespace internal {
        template <typename F, typename... Args>
        struct BLOCKSCI_EXPORT is_callable {
            template <typename U>
            static auto test(U* p) -> decltype((*p)(std::declval<Args>()...), void(), std::true_type());
            
            template <typename U>
            static auto test(...) -> decltype(std::false_type());
            
            static constexpr bool value = decltype(test<F>(nullptr))::value;
        };
        
        template <typename ResultType, typename It, typename MapFunc, typename ReduceFunc>
        ResultType BLOCKSCI_EXPORT mapReduceBlocksImp(It begin, It end, MapFunc mapFunc, ReduceFunc reduceFunc, int segmentBeginNum) {
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

    /** Represents an iterable collection of contiguous Block objects */
    class BLOCKSCI_EXPORT BlockRange {
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
            self_type operator+(difference_type i) const { self_type tmp = *this; tmp.height += i; return tmp; }
            self_type operator-(difference_type i) const { self_type tmp = *this; tmp.height -= i; return tmp; }
            
            value_type operator*() const { return Block(height, *access); }
            value_type operator[](difference_type i) const { return Block(height + i, *access); }
            
            bool operator==(const self_type& rhs) const { return height == rhs.height; }
            bool operator!=(const self_type& rhs) const { return height != rhs.height; }
            bool operator<(const self_type& rhs) const { return height < rhs.height; }
            bool operator>(const self_type& rhs) const { return height > rhs.height; }
            bool operator<=(const self_type& rhs) const { return height <= rhs.height; }
            bool operator>=(const self_type& rhs) const { return height >= rhs.height; }
            
            BlockHeight operator-(const self_type& it) const {return height - it.height;}
        private:
            BlockHeight height;
            DataAccess *access;
        };
        
        struct Slice {
            BlockHeight start;
            BlockHeight stop;
        };
        
        BlockRange() = default;
        BlockRange(const Slice &x, DataAccess *access_) : sl(x), access(access_) {}
        
        iterator begin() const {
            return {sl.start, access};
        }
        
        iterator end() const {
            return {sl.stop, access};
        }
        
        auto operator[](BlockHeight height) const {
            return Block(sl.start + height, *access);
        }
        
        BlockRange operator[](const Slice &slice) const {
            return {{sl.start + slice.start, sl.start + slice.stop}, access};
        }
        
        BlockHeight size() const {
            return sl.stop - sl.start;
        }
        
        uint32_t firstTxIndex() const {
            return this->operator[](0).firstTxIndex();
        }
        uint32_t endTxIndex() const {
            return this->operator[](size() - 1).endTxIndex();
        }
        
        template <typename ResultType, typename MapFunc, typename ReduceFunc>
        std::enable_if_t<internal::is_callable<MapFunc, BlockRange, int>::value, ResultType>
        mapReduce(MapFunc mapFunc, ReduceFunc reduceFunc) {
            auto segments = segment(std::thread::hardware_concurrency());
            return internal::mapReduceBlocksImp<ResultType>(segments.begin(), segments.end(), mapFunc, reduceFunc, 0);
        }
        
        template <typename ResultType, typename MapFunc, typename ReduceFunc>
        std::enable_if_t<internal::is_callable<MapFunc, BlockRange>::value, ResultType>
        mapReduce(MapFunc mapFunc, ReduceFunc reduceFunc) {
            auto segments = segment(std::thread::hardware_concurrency());
            return internal::mapReduceBlocksImp<ResultType>(segments.begin(), segments.end(), [&](const BlockRange &blocks, int) { return mapFunc(blocks); }, reduceFunc, 0);
        }
        
        template <typename ResultType, typename MapFunc, typename ReduceFunc>
        std::enable_if_t<internal::is_callable<MapFunc, Block>::value, ResultType>
        mapReduce(MapFunc mapFunc, ReduceFunc reduceFunc) {
            auto mapF = [&](const BlockRange &segment) {
                ResultType res{};
                for (auto block : segment) {
                    auto mapped = mapFunc(block);
                    res = reduceFunc(res, mapped);
                }
                return res;
            };
            return mapReduce<ResultType>(mapF, reduceFunc);
        }
        
        template <typename ResultType, typename MapFunc, typename ReduceFunc>
        std::enable_if_t<internal::is_callable<MapFunc, Transaction>::value, ResultType>
        mapReduce(MapFunc mapFunc, ReduceFunc reduceFunc) {
            auto mapF = [&](const Block &block) {
                ResultType res{};
                for (auto tx : block) {
                    auto mapped = mapFunc(tx);
                    res = reduceFunc(res, mapped);
                }
                return res;
            };
            
            return mapReduce<ResultType>(mapF, reduceFunc);
        }
        
        template <typename MapType>
        std::vector<MapType> map(const std::function<MapType(const Block &)> &mapFunc) {
            auto mapF = [&](const BlockRange &segment) {
                std::vector<MapType> vec;
                vec.reserve(segment.size());
                for (auto block : segment) {
                    vec.push_back(mapFunc(block));
                }
                return vec;
            };
            
            auto reduceFunc = [](std::vector<MapType> &vec1, std::vector<MapType> &vec2) -> std::vector<MapType> & {
                vec1.reserve(vec1.size() + vec2.size());
                vec1.insert(vec1.end(), std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));
                return vec1;
            };
            
            return mapReduce<std::vector<MapType>>(mapF, reduceFunc);
        }
        
        std::vector<Block> filter(std::function<bool(const Block &block)> testFunc);
        std::vector<Transaction> filter(std::function<bool(const Transaction &tx)> testFunc);

        // Returns a vector of [start, stop) intervals splitting the chain into segments with approximately the same number of segments
        std::vector<BlockRange> segment(unsigned int segmentCount) const;
        
        Slice sl;
        
        DataAccess &getAccess() { return *access; }
        
    private:
        DataAccess *access;
        
        
    };
    
    inline std::vector<Transaction> BLOCKSCI_EXPORT getTransactionsIncludingOutput(BlockRange &chain, AddressType::Enum type) {
        return chain.filter([type](const Transaction &tx) {
            return includesOutputOfType(tx, type);
        });
    }
    
    inline BlockRange::iterator::self_type BLOCKSCI_EXPORT operator+(BlockRange::iterator::difference_type i, const BlockRange::iterator &it) {
        return it + i;
    }
    
    CPP_assert(ranges::bidirectional_range<BlockRange>);
    CPP_assert(ranges::bidirectional_iterator<BlockRange::iterator>);
    CPP_assert(ranges::sized_sentinel_for<BlockRange::iterator, BlockRange::iterator>);
    CPP_assert(ranges::totally_ordered<BlockRange::iterator>);
    CPP_assert(ranges::random_access_iterator<BlockRange::iterator>);
    CPP_assert(ranges::random_access_range<BlockRange>);
    CPP_assert(ranges::sized_range<BlockRange>);
} // namespace blocksci


#endif /* block_range_hpp */
