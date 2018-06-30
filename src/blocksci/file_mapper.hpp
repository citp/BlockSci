//
//  file_mapper.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/23/17.
//
//

#ifndef file_mapper_hpp
#define file_mapper_hpp

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem.hpp>

#include <boost/range/adaptor/transformed.hpp>

#include <boost/optional.hpp>

#include <array>
#include <vector>
#include <iostream>
#include <stdio.h>

namespace blocksci {
    
    template<typename U>
    struct add_const_ptr {
        using type = std::add_pointer_t<std::add_const_t<U>>;
    };
    
    template<typename... Types>
    struct add_const_ptr<std::tuple<Types...>> {
        using type = std::tuple<std::add_pointer_t<std::add_const_t<Types>>...>;
    };
    template<typename... Types>
    using add_const_ptr_t = typename add_const_ptr<Types...>::type;
    
    
    template<typename U>
    struct add_ptr {
        using type = std::add_pointer_t<U>;
    };
    
    template<typename... Types>
    struct add_ptr<std::tuple<Types...>> {
        using type = std::tuple<std::add_pointer_t<Types>...>;
    };
    template<typename... Types>
    using add_ptr_t = typename add_ptr<Types...>::type;
    
    
    struct SimpleFileMapperBase {
    protected:
        boost::iostreams::mapped_file file;
        size_t fileEnd;
    public:
        boost::filesystem::path path;
        // boost::interprocess::read_only
        SimpleFileMapperBase(boost::filesystem::path path_, boost::iostreams::mapped_file::mapmode mode) : fileEnd(0), path(path_) {
            path += ".dat";
            
            if (boost::filesystem::exists(path)) {
                fileEnd = boost::filesystem::file_size(path);
                if (fileEnd != 0) {
                    
                    file.open(path, mode);
                }
            }
        }
        
        bool isGood() const {
            return file.is_open();
        }
        
        const char *getDataAtOffset(size_t offset) const {
            return reinterpret_cast<const char *>(file.const_data()) + offset;
        }
        
        size_t size() const {
            return fileEnd;
        }
    };
    
    template<boost::iostreams::mapped_file::mapmode mode = boost::iostreams::mapped_file::mapmode::readonly>
    struct SimpleFileMapper;
    
    template <>
    struct SimpleFileMapper<boost::iostreams::mapped_file::mapmode::readonly> : public SimpleFileMapperBase {
        SimpleFileMapper(boost::filesystem::path path) : SimpleFileMapperBase(path, boost::iostreams::mapped_file::mapmode::readonly) {}
    };
    
    template <>
    struct SimpleFileMapper<boost::iostreams::mapped_file::mapmode::readwrite> : public SimpleFileMapperBase {
        static constexpr size_t bufferSize = 50000000;
        std::unique_ptr<std::array<char, bufferSize>> buffer;
        size_t bufferOffset;
        
        SimpleFileMapper(boost::filesystem::path path) : SimpleFileMapperBase(path, boost::iostreams::mapped_file::mapmode::readwrite), buffer(std::make_unique<std::array<char, bufferSize>>()), bufferOffset(0) {}
        
        ~SimpleFileMapper() {
            clearBuffer();
        }
        
    protected:
        
        template<typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
        bool writeImp(const T &t) {
            bool bufferCleared = bufferOffset + sizeof(t) > bufferSize;
            if (bufferCleared) {
                clearBuffer();
            }
            std::memcpy(buffer->data() + bufferOffset, &t, sizeof(t));
            bufferOffset += sizeof(T);
            return bufferCleared;
        }
        
    public:
        
        void clearBuffer() {
            if (bufferOffset > 0) {
                if (!file.is_open()) {
                    boost::iostreams::mapped_file_params params{path.native()};
                    params.new_file_size = bufferOffset;
                    params.flags = boost::iostreams::mapped_file::readwrite;
                    file.open(params);
                } else {
                    file.resize(fileEnd + bufferOffset);
                }
                memcpy(file.data() + fileEnd, &(*buffer)[0], bufferOffset);
                fileEnd += bufferOffset;
                bufferOffset = 0;
            }
        }
        
        template <typename T>
        void update(size_t offset, const T &t) {
            memcpy(reinterpret_cast<char *>(file.data()) + offset, &t, sizeof(T));
        }
        
        char *getDataAtOffset(size_t offset) {
            if (offset < fileEnd) {
                return file.data() + offset;
            } else {
                return &(*buffer)[offset - fileEnd];
            }
        }
        
        const char *getDataAtOffset(size_t offset) const {
            if (offset < fileEnd) {
                return file.const_data() + offset;
            } else {
                return &(*buffer)[offset - fileEnd];
            }
        }
        
        size_t size() const {
            return SimpleFileMapperBase::size() + bufferOffset;
        }
        
        void truncate(size_t offset) {
            boost::filesystem::resize_file(path, offset);
        }
    };
    
    template<boost::iostreams::mapped_file::mapmode mode = boost::iostreams::mapped_file::mapmode::readonly>
    struct ArbitraryFileMapper : public SimpleFileMapper<mode> {
        using SimpleFileMapper<mode>::SimpleFileMapper;
        
        template<typename T>
        bool write(const T &t) {
            return SimpleFileMapper<mode>::writeImp(t);
        }
        
        template<class It>
        bool write(It it, It end) {
            auto distance = static_cast<uint32_t>(std::distance(it, end));
            bool clearedBuffer = SimpleFileMapper<mode>::writeImp(distance);
            for (; it != end; ++it) {
                clearedBuffer |= SimpleFileMapper<mode>::writeImp(*it);
            }
            if (clearedBuffer) {
                SimpleFileMapper<mode>::clearBuffer();
            }
            return clearedBuffer;
        }
    };
    
    template <typename T, boost::iostreams::mapped_file::mapmode mode = boost::iostreams::mapped_file::mapmode::readonly>
    struct FixedSizeFileMapper : public SimpleFileMapper<mode> {
        using SimpleFileMapper<mode>::SimpleFileMapper;
        
        add_const_ptr_t<T> getData(size_t index) const {
            const char *pos = SimpleFileMapper<mode>::getDataAtOffset(index * sizeof(T));
            return reinterpret_cast<add_const_ptr_t<T>>(pos);
        }
        
        add_ptr_t<T> getData(size_t index) {
            char *pos = SimpleFileMapper<mode>::getDataAtOffset(index * sizeof(T));
            return reinterpret_cast<add_ptr_t<T>>(pos);
        }
        
        void update(uint32_t index, const T &t) {
            SimpleFileMapper<mode>::update(index * sizeof(T), t);
        }
        
        bool write(const T &t) {
            return SimpleFileMapper<mode>::writeImp(t);
        }
        
        size_t size() const {
            return SimpleFileMapper<mode>::size() / sizeof(T);
        }
        
        void truncate(size_t index) {
            boost::filesystem::resize_file(SimpleFileMapper<mode>::path, index * sizeof(T));
        }
        
        boost::iterator_range<add_const_ptr_t<T>> getRange() const {
            return boost::make_iterator_range_n(getData(0), size());
        }
        
        template<typename Test>
        std::vector<uint32_t> findAll(Test test) const {
            auto itemCount = size();
            std::vector<uint32_t> indexes;
            uint32_t index = 0;
            while (index < itemCount) {
                auto nextItem = getData(index);
                if (test(*nextItem)) {
                    indexes.push_back(index);
                }
                index++;
            }
            return indexes;
        }
        
        template<typename Test>
        boost::optional<uint32_t> find(Test test) const {
            auto itemCount = size();
            std::vector<uint32_t> indexes;
            uint32_t index = 0;
            while (index < itemCount) {
                auto nextItem = getData(index);
                if (test(*nextItem)) {
                    return index;
                }
                index++;
            }
            return boost::none;
        }
    };
    
    template <size_t indexCount>
    using FileIndex = std::array<uint64_t, indexCount>;
    
    
    template <typename Tup, typename A, std::size_t ...I>
    Tup tuple_cast_helper(A t1, std::index_sequence<I...>) {
        return std::make_tuple( reinterpret_cast<std::tuple_element_t<I,Tup>>(std::get<I>(t1))...);
    }
    
    template <typename ...A, size_t N>
    add_const_ptr_t<std::tuple<A...>> tuple_cast(std::array<const char *, N> t1) {
        static_assert(sizeof...(A) == N, "The tuple sizes must be the same");
        return tuple_cast_helper<add_const_ptr_t<std::tuple<A...>>>( t1, std::make_index_sequence<N>{} );
    }
    
    
    template <size_t indexNum = 0, boost::iostreams::mapped_file::mapmode mode = boost::iostreams::mapped_file::mapmode::readonly, typename... T>
    struct IndexedFileExpander;
    
    template <boost::iostreams::mapped_file::mapmode mode = boost::iostreams::mapped_file::mapmode::readonly, typename... T>
    struct IndexedFileMapper : public ArbitraryFileMapper<mode> {
    private:
        template<size_t indexNum>
        using nth_element = std::tuple_element_t<indexNum, std::tuple<T...>>;
        static constexpr size_t indexCount = sizeof...(T);
        FixedSizeFileMapper<FileIndex<indexCount>, mode> indexFile;
    public:
        IndexedFileMapper(boost::filesystem::path pathPrefix) : ArbitraryFileMapper<mode>(boost::filesystem::path(pathPrefix).concat("_data")), indexFile(boost::filesystem::path(pathPrefix).concat("_index")) {
        }
        
        template<size_t indexNum = 0>
        uint64_t getOffset(size_t index) const {
            static_assert(indexNum < sizeof...(T), "Trying to fetch index out of bounds");
            auto indexData = indexFile.getData(index);
            return (*indexData)[indexNum];
        }
        
        FileIndex<sizeof...(T)> getOffsets(size_t index) const {
            return *indexFile.getData(index);
        }
        
        size_t size() const {
            return indexFile.size();
        }
        
        void truncate(size_t index) {
            auto offsets = getOffsets(index);
            indexFile.truncate(index);
            uint64_t minOffset = 0;
            for (auto &offset : offsets) {
                if (minOffset == 0) {
                    minOffset = offset;
                } else if (offset != 0) {
                    minOffset = std::min(minOffset, offset);
                }
            }
            ArbitraryFileMapper<mode>::truncate(minOffset);
        }
        
        template<size_t indexNum = 0>
        boost::iterator_range<add_const_ptr_t<nth_element<indexNum>>> getRange() const {
            IndexedFileExpander<indexNum, mode, T...> expanded(*this);
            return indexFile.getRange() | boost::adaptors::transformed(expanded);
        }
        
        std::array<const char *, sizeof...(T)> getPointersAtIndex(size_t index) const {
            auto offsets = getOffsets(index);
            std::array<const char *, sizeof...(T)> pointers;
            for (size_t i = 0; i < sizeof...(T); i++) {
                pointers[i] = ArbitraryFileMapper<mode>::getDataAtOffset(offsets[i]);
            }
            return pointers;
        }
        
        template<size_t indexNum = 0>
        const char *getPointerAtIndex(size_t index) const {
            return ArbitraryFileMapper<mode>::getDataAtOffset(getOffset<indexNum>(index));
        }
        
        template<size_t indexNum = 0>
        char *getPointerAtIndex(size_t index) {
            return ArbitraryFileMapper<mode>::getDataAtOffset(getOffset<indexNum>(index));
        }
        
        template<size_t indexNum = 0>
        void update(uint32_t index, const nth_element<indexNum> &t) {
            ArbitraryFileMapper<mode>::update(getOffset<indexNum>(index), t);
        }
        
        bool writeIndexGroup() {
            FileIndex<indexCount> fileIndex;
            fileIndex[0] = ArbitraryFileMapper<mode>::size();
            for(size_t i = 1; i < indexCount; i++) {
                fileIndex[i] = 0;
            }
            return indexFile.write(fileIndex);
        }
        
        template<size_t indexNum>
        void updateIndexGroup(uint32_t addressNum) {
            static_assert(indexNum < indexCount, "Index out of range");
            auto &fileIndex = *indexFile.getData(addressNum);
            fileIndex[indexNum] = ArbitraryFileMapper<mode>::size();
        }
        
        template<size_t indexNum = 0>
        add_const_ptr_t<nth_element<indexNum>> getDataAtIndex(size_t index) const {
            return reinterpret_cast<add_const_ptr_t<nth_element<indexNum>>>(getPointerAtIndex<indexNum>(index));
        }
        
        template<class A=add_const_ptr_t<std::tuple<T...>>>
        std::enable_if_t<(sizeof...(T) > 1), A>
        getData(size_t index) const {
            auto pointers = getPointersAtIndex(index);
            return tuple_cast<T...>(pointers);
        }
        
        template<class A=add_const_ptr_t<nth_element<0>>>
        std::enable_if_t<(sizeof...(T) == 1), A>
        getData(size_t index) const {
            return getDataAtIndex<0>(index);
        }
        
    };
    
    template <size_t indexNum, boost::iostreams::mapped_file::mapmode mode, typename... T>
    struct IndexedFileExpander
    {
        typedef decltype(std::declval<IndexedFileMapper<mode, T...>>().getData(0)) result_type;
        
        const IndexedFileMapper<mode, T...> &file;
        
        IndexedFileExpander(const IndexedFileMapper<mode, T...> &file_) : file(file_) {}
        result_type operator()(const FileIndex<sizeof...(T)> &index) const {
            return file.getData(index);
        }
    };
}

#endif /* file_mapper_hpp */
