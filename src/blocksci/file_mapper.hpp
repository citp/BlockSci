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
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>

#include <boost/range/adaptor/transformed.hpp>

#include <boost/optional/optional_fwd.hpp>

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
    
    using OffsetType = uint64_t;
    constexpr OffsetType InvalidFileIndex = std::numeric_limits<OffsetType>::max();
    
    struct SimpleFileMapperBase {
        using FileType = boost::iostreams::mapped_file;
    protected:
        FileType file;
        size_t fileEnd;
    public:
        boost::filesystem::path path;
        FileType::mapmode fileMode;
        
        void reload() {
            if (boost::filesystem::exists(path)) {
                auto newSize = boost::filesystem::file_size(path);
                if (newSize != fileEnd) {
                    if (file.is_open()) {
                        file.close();
                    }
                    file.open(path, fileMode);
                    fileEnd = newSize;
                }
            } else {
                if (file.is_open()) {
                    file.close();
                }
                fileEnd = 0;
            }
        }
        
        SimpleFileMapperBase(boost::filesystem::path path_, FileType::mapmode mode) : fileEnd(0), path(path_), fileMode(mode) {
            path += ".dat";
            
            if (boost::filesystem::exists(path)) {
                fileEnd = boost::filesystem::file_size(path);
                if (fileEnd != 0) {
                    file.open(path, mode);
                }
            }
        }
        
        SimpleFileMapperBase(const SimpleFileMapperBase &) = delete;
        SimpleFileMapperBase &operator=(const SimpleFileMapperBase &) = delete;
        
        bool isGood() const {
            return file.is_open();
        }
        
        void clearBuffer() {}
        
        const char *getDataAtOffset(OffsetType offset) const {
            if (offset < size()) {
                return reinterpret_cast<const char *>(file.const_data()) + offset;
            } else {
                return nullptr;
            }
            
        }
        
        size_t size() const {
            return fileEnd;
        }
        
        size_t fileSize() const {
            return boost::filesystem::file_size(path);
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
        static constexpr size_t maxBufferSize = 50000000;
        std::vector<char> buffer;
        
        SimpleFileMapper(boost::filesystem::path path) : SimpleFileMapperBase(path, boost::iostreams::mapped_file::mapmode::readwrite) {}
        
        ~SimpleFileMapper() {
            clearBuffer();
        }
        
    protected:
        
        template<typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
        bool writeImp(const T &t) {
            auto start = reinterpret_cast<const char *>(&t);
            buffer.insert(buffer.end(), start, start + sizeof(t));
            
            bool bufferFull = buffer.size() > maxBufferSize;
            if (bufferFull) {
                clearBuffer();
            }
            return bufferFull;
        }
        
    public:
        
        void clearBuffer() {
            if (buffer.size() > 0) {
                if (!file.is_open()) {
                    boost::iostreams::mapped_file_params params{path.native()};
                    params.new_file_size = static_cast<decltype(params.new_file_size)>(buffer.size());
                    params.flags = boost::iostreams::mapped_file::readwrite;
                    file.open(params);
                } else {
                    file.resize(static_cast<int64_t>(fileEnd + buffer.size()));
                }
                memcpy(file.data() + fileEnd, buffer.data(), buffer.size());
                fileEnd += buffer.size();
                buffer.clear();
            }
        }
        
        template <typename T>
        void update(OffsetType offset, const T &t) {
            memcpy(reinterpret_cast<char *>(file.data()) + offset, &t, sizeof(T));
        }
        
        char *getDataAtOffset(OffsetType offset) {
            assert(offset < fileEnd + buffer.size() || offset == InvalidFileIndex);
            if (offset == InvalidFileIndex) {
                return nullptr;
            } else if (offset < fileEnd) {
                return file.data() + offset;
            } else {
                return buffer.data() + (offset - fileEnd);
            }
        }
        
        const char *getDataAtOffset(OffsetType offset) const {
            assert(offset < fileEnd + buffer.size() || offset == InvalidFileIndex);
            if (offset == InvalidFileIndex) {
                return nullptr;
            } else if (offset < fileEnd) {
                return file.const_data() + offset;
            } else {
                return buffer.data() + (offset - fileEnd);
            }
        }
        
        size_t size() const {
            return SimpleFileMapperBase::size() + buffer.size();
        }
        
        void truncate(OffsetType offset) {
            if (offset < SimpleFileMapperBase::size()) {
                buffer.clear();
                boost::filesystem::resize_file(path, offset);
                reload();
            } else if (offset < size()) {
                auto bufferToSave = offset - SimpleFileMapperBase::size();
                buffer.resize(bufferToSave);
            } else if (offset > size()) {
                clearBuffer();
                if (!boost::filesystem::exists(path)) {
                    boost::filesystem::fstream{path, std::fstream::out | std::fstream::binary};
                }
                boost::filesystem::resize_file(path, offset);
                reload();
            }
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
            auto rangeLength = sizeof(typename std::iterator_traits<It>::value_type) * distance;
            bool clearedBuffer = SimpleFileMapper<mode>::writeImp(distance);
            for (; it != end; ++it) {
                clearedBuffer |= SimpleFileMapper<mode>::writeImp(*it);
            }
            auto leftover = rangeLength % sizeof(distance);
            if (leftover != 0) {
                auto filler = sizeof(distance) - leftover;
                for (size_t i = 0; i < filler; i++) {
                    clearedBuffer |= SimpleFileMapper<mode>::writeImp(static_cast<uint8_t>(0));
                }
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
            assert(index < size());
            const char *pos = SimpleFileMapper<mode>::getDataAtOffset(index * sizeof(T));
            return reinterpret_cast<add_const_ptr_t<T>>(pos);
        }
        
        add_ptr_t<T> getData(size_t index) {
            assert(index < size());
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
        
        size_t fileSize() const {
            return SimpleFileMapper<mode>::fileSize() / sizeof(T);
        }
        
        void truncate(size_t index) {
            SimpleFileMapper<mode>::truncate(index * sizeof(T));
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
    using FileIndex = std::array<OffsetType, indexCount>;
    
    
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
        
        void reload() {
            clearBuffer();
            indexFile.reload();
            ArbitraryFileMapper<mode>::reload();
        }
        
        void clearBuffer() {
            indexFile.clearBuffer();
            ArbitraryFileMapper<mode>::clearBuffer();
        }
        
        template<size_t indexNum = 0>
        OffsetType getOffset(size_t offset) const {
            static_assert(indexNum < sizeof...(T), "Trying to fetch index out of bounds");
            auto indexData = indexFile.getData(offset);
            return (*indexData)[indexNum];
        }
        
        FileIndex<sizeof...(T)> getOffsets(uint32_t index) const {
            return *indexFile.getData(index);
        }
        
        size_t size() const {
            return indexFile.size();
        }
        
        size_t fileSize() const {
            return indexFile.fileSize();
        }
        
        void truncate(uint32_t index) {
            auto offsets = getOffsets(index);
            indexFile.truncate(index);
            uint64_t minOffset = InvalidFileIndex;
            for (auto &offset : offsets) {
                 minOffset = std::min(minOffset, offset);
            }
            ArbitraryFileMapper<mode>::truncate(minOffset);
        }
        
        template<size_t indexNum = 0>
        boost::iterator_range<add_const_ptr_t<nth_element<indexNum>>> getRange() const {
            IndexedFileExpander<indexNum, mode, T...> expanded(*this);
            return indexFile.getRange() | boost::adaptors::transformed(expanded);
        }
        
        std::array<const char *, sizeof...(T)> getPointersAtIndex(uint32_t index) const {
            auto offsets = getOffsets(index);
            std::array<const char *, sizeof...(T)> pointers;
            for (size_t i = 0; i < sizeof...(T); i++) {
                pointers[i] = ArbitraryFileMapper<mode>::getDataAtOffset(offsets[i]);
            }
            return pointers;
        }
        
        template<size_t indexNum = 0>
        const char *getPointerAtIndex(uint32_t index) const {
            return ArbitraryFileMapper<mode>::getDataAtOffset(getOffset<indexNum>(index));
        }
        
        template<size_t indexNum = 0>
        char *getPointerAtIndex(uint32_t index) {
            assert(index < size());
            auto offset = getOffset<indexNum>(index);
            return ArbitraryFileMapper<mode>::getDataAtOffset(offset);
        }
        
        template<size_t indexNum = 0>
        void update(uint32_t index, const nth_element<indexNum> &t) {
            ArbitraryFileMapper<mode>::update(getOffset<indexNum>(index), t);
        }
        
        bool writeIndexGroup() {
            FileIndex<indexCount> fileIndex;
            fileIndex[0] = ArbitraryFileMapper<mode>::size();
            for(size_t i = 1; i < indexCount; i++) {
                fileIndex[i] = InvalidFileIndex;
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
        add_const_ptr_t<nth_element<indexNum>> getDataAtIndex(uint32_t index) const {
            assert(index < size());
            return reinterpret_cast<add_const_ptr_t<nth_element<indexNum>>>(getPointerAtIndex<indexNum>(index));
        }
        
        template<size_t indexNum = 0>
        add_ptr_t<nth_element<indexNum>> getDataAtIndex(uint32_t index) {
            assert(index < size());
            auto ptr = reinterpret_cast<add_ptr_t<nth_element<indexNum>>>(getPointerAtIndex<indexNum>(index));
            assert(ptr != nullptr);
            return ptr;
        }
        
        template<class A=add_const_ptr_t<std::tuple<T...>>>
        std::enable_if_t<(sizeof...(T) > 1), A>
        getData(uint32_t index) const {
            assert(index < size());
            auto pointers = getPointersAtIndex(index);
            return tuple_cast<T...>(pointers);
        }
        
        template<class A=add_const_ptr_t<nth_element<0>>>
        std::enable_if_t<(sizeof...(T) == 1), A>
        getData(uint32_t index) const {
            assert(index < size());
            return getDataAtIndex<0>(index);
        }
        
        template<class A=add_ptr_t<std::tuple<T...>>>
        std::enable_if_t<(sizeof...(T) > 1), A>
        getData(uint32_t index) {
            assert(index < size());
            auto pointers = getPointersAtIndex(index);
            return tuple_cast<T...>(pointers);
        }
        
        template<class A=add_ptr_t<nth_element<0>>>
        std::enable_if_t<(sizeof...(T) == 1), A>
        getData(uint32_t index) {
            assert(index < size());
            auto data = getDataAtIndex<0>(index);
            assert(data != nullptr);
            return data;
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
