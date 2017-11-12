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
    private:
        const char *constData;
        
        void openFile(size_t size) {
            fileEnd = size;
            if (fileEnd != 0) {
                file.open(path, fileMode);
                constData = file.const_data();
            }
        }
        
    protected:
        FileType file;
        size_t fileEnd;
    public:
        boost::filesystem::path path;
        FileType::mapmode fileMode;
        
        void reload() {
            if (boost::filesystem::exists(path)) {
                auto newSize = fileSize();
                if (newSize != fileEnd) {
                    if (file.is_open()) {
                        file.close();
                    }
                    openFile(newSize);
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
                openFile(fileSize());
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
                return constData + offset;
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
    
    template <typename MainType>
    class ArbitraryLengthData {
        std::vector<char> rawData;
        
    public:
        ArbitraryLengthData(const MainType &head) {
            rawData.reserve(head.realSize());
            add(head);
        }
        
        template <typename T>
        void add(const T &t) {
            auto dataPtr = reinterpret_cast<const char *>(&t);
            rawData.insert(rawData.end(), dataPtr, dataPtr + sizeof(t));
        }
        
        template <typename It>
        void add(It begin, It end) {
            for (; begin != end; ++begin) {
                add(*begin);
            }
        }
        
        
        size_t size() const {
            return rawData.size();
        }
        
        const char *dataView() const {
            return rawData.data();
        }
    };
    
    template <>
    struct SimpleFileMapper<boost::iostreams::mapped_file::mapmode::readwrite> : public SimpleFileMapperBase {
        static constexpr size_t maxBufferSize = 50000000;
        std::vector<char> buffer;
        static constexpr auto mode = boost::iostreams::mapped_file::mapmode::readwrite;
        
        SimpleFileMapper(boost::filesystem::path path) : SimpleFileMapperBase(path, boost::iostreams::mapped_file::mapmode::readwrite), writePos(size()) {}
        
        SimpleFileMapper(const SimpleFileMapper &) = delete;
        SimpleFileMapper &operator=(const SimpleFileMapper &) = delete;
        SimpleFileMapper(SimpleFileMapper &&) = delete;
        SimpleFileMapper &operator=(SimpleFileMapper &&) = delete;
        
        ~SimpleFileMapper() {
            clearBuffer();
        }
        
    private:
        OffsetType writePos;
        
        char *getWritePos() {
            if (writePos < fileEnd) {
                return reinterpret_cast<char *>(file.data()) + writePos;
            } else if (writePos < fileEnd + buffer.size()) {
                return reinterpret_cast<char *>(buffer.data()) + (writePos - fileEnd);
            } else {
                return nullptr;
            }
        }
        
        size_t writeSpace() const {
            if (writePos < fileEnd) {
                return fileEnd - writePos;
            } else if (writePos < fileEnd + buffer.size()) {
                return (writePos - fileEnd) - buffer.size();
            } else {
                return 0;
            }
        }
        
    public:
        
        void reload() {
            clearBuffer();
            SimpleFileMapperBase::reload();
        }
        
        OffsetType getWriteOffset() const {
            return writePos;
        }
        
        bool write(const char *valuePos, size_t amountToWrite) {
            if (writePos < fileEnd) {
                auto writeAmount = std::min(amountToWrite, writeSpace());
                memcpy(getWritePos(), valuePos, writeAmount);
                amountToWrite -= writeAmount;
                writePos += writeAmount;
                valuePos += writeAmount;
                if (amountToWrite == 0) {
                    return false;
                }
            }
            
            if (writePos >= fileEnd && writePos < buffer.size()) {
                auto writeAmount = std::min(amountToWrite, writeSpace());
                memcpy(getWritePos(), valuePos, writeAmount);
                amountToWrite -= writeAmount;
                writePos += writeAmount;
                valuePos += writeAmount;
                
                if (amountToWrite == 0) {
                    return false;
                }
            }
            
            assert(writePos == fileEnd + buffer.size());
            
            buffer.insert(buffer.end(), valuePos, valuePos + amountToWrite);
            writePos += amountToWrite;
            bool bufferFull = buffer.size() > maxBufferSize;
            if (bufferFull) {
                clearBuffer();
            }
            return bufferFull;
        }
        
        template<typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
        bool write(const T &t) {
            return write(reinterpret_cast<const char *>(&t), sizeof(t));
        }
        
        template<typename T>
        bool write(const ArbitraryLengthData<T> &t) {
            return write(t.dataView(), t.size());
        }
        
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
        
        void seekEnd() {
            writePos = fileEnd + buffer.size();
        }
        
        void seek(size_t offset) {
            writePos = offset;
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
                    boost::filesystem::fstream s{path, std::fstream::out | std::fstream::binary};
                    s.seekp(offset - 1);
                    s.write("", 1);
                } else {
                    boost::filesystem::resize_file(path, offset);
                }
                reload();
            }
        }
    };
    
    template<boost::iostreams::mapped_file::mapmode mode = boost::iostreams::mapped_file::mapmode::readonly>
    struct ArbitraryFileMapper : public SimpleFileMapper<mode> {
        using SimpleFileMapper<mode>::SimpleFileMapper;
        
        template<typename T>
        bool write(const T &t) {
            return SimpleFileMapper<mode>::writeImp(t, t.realSize());
        }
    };
    
    template <typename T, boost::iostreams::mapped_file::mapmode mode = boost::iostreams::mapped_file::mapmode::readonly>
    struct FixedSizeFileMapper {
    private:
        SimpleFileMapper<mode> dataFile;
        size_t getPos(uint32_t index) const {
            return static_cast<size_t>(index) * sizeof(T);
        }
    public:
        
        FixedSizeFileMapper(boost::filesystem::path path) : dataFile(path) {}
        
        add_const_ptr_t<T> getData(size_t index) const {
            assert(index < size());
            const char *pos = dataFile.getDataAtOffset(getPos(index));
            return reinterpret_cast<add_const_ptr_t<T>>(pos);
        }
        
        add_ptr_t<T> getData(size_t index) {
            assert(index < size());
            char *pos = dataFile.getDataAtOffset(getPos(index));
            return reinterpret_cast<add_ptr_t<T>>(pos);
        }
        
        void seekEnd() {
            dataFile.seekEnd();
        }
        
        void seek(uint32_t index) {
            dataFile.seek(getPos(index));
        }
        
        void reload() {
            dataFile.reload();
        }
        
        void clearBuffer() {
            dataFile.clearBuffer();
        }
        
        bool write(const T &t) {
            return dataFile.write(t);
        }
        
        size_t size() const {
            return dataFile.size() / sizeof(T);
        }
        
        size_t fileSize() const {
            return dataFile.fileSize() / sizeof(T);
        }
        
        void truncate(size_t index) {
            dataFile.truncate(getPos(index));
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
    struct IndexedFileMapper {
    private:
        template<size_t indexNum>
        using nth_element = std::tuple_element_t<indexNum, std::tuple<T...>>;
        static constexpr size_t indexCount = sizeof...(T);
        SimpleFileMapper<mode> dataFile;
        FixedSizeFileMapper<FileIndex<indexCount>, mode> indexFile;
        
        void writeNewImp(const char *valuePos, size_t amountToWrite) {
            FileIndex<indexCount> fileIndex;
            fileIndex[0] = dataFile.getWriteOffset();
            for(size_t i = 1; i < indexCount; i++) {
                fileIndex[i] = InvalidFileIndex;
            }
            indexFile.write(fileIndex);
            dataFile.write(valuePos, amountToWrite);
        }
        
        template<size_t indexNum>
        void writeUpdateImp(uint32_t addressNum, const char *valuePos, size_t amountToWrite) {
            static_assert(indexNum > 0, "Must call write without addressNum for first element");
            static_assert(indexNum < indexCount, "Index out of range");
            auto &fileIndex = *indexFile.getData(addressNum);
            fileIndex[indexNum] = dataFile.getWriteOffset();
            dataFile.write(valuePos, amountToWrite);
        }
        
    public:
        IndexedFileMapper(boost::filesystem::path pathPrefix) : dataFile(boost::filesystem::path(pathPrefix).concat("_data")), indexFile(boost::filesystem::path(pathPrefix).concat("_index")) {
        }
        
        void reload() {
            indexFile.reload();
            dataFile.reload();
        }
        
        void clearBuffer() {
            indexFile.clearBuffer();
            dataFile.clearBuffer();
        }
        
        template<size_t indexNum = 0>
        OffsetType getOffset(uint32_t index) const {
            static_assert(indexNum < sizeof...(T), "Trying to fetch index out of bounds");
            auto indexData = indexFile.getData(index);
            auto offset = (*indexData)[indexNum];
            assert(offset < dataFile.size() || offset == InvalidFileIndex);
            return offset;
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
            dataFile.truncate(offsets[0]);
        }
        
        void seekEnd() {
            indexFile.seekEnd();
            dataFile.seekEnd();
        }
        
        void seek(uint32_t index, size_t dataOffset) {
            indexFile.seek(index);
            dataFile.seek(dataOffset);
        }

        void grow(uint32_t indexCount, uint64_t dataSize) {
            indexFile.truncate(indexFile.size() + indexCount);
            dataFile.truncate(dataFile.size() + dataSize);
        }
        
        template<size_t indexNum = 0>
        auto getRange() const {
            IndexedFileExpander<indexNum, mode, T...> expanded(*this);
            return indexFile.getRange() | boost::adaptors::transformed(expanded);
        }
        
        std::array<const char *, sizeof...(T)> getPointersAtIndex(uint32_t index) const {
            auto offsets = getOffsets(index);
            std::array<const char *, sizeof...(T)> pointers;
            for (size_t i = 0; i < sizeof...(T); i++) {
                pointers[i] = dataFile.getDataAtOffset(offsets[i]);
            }
            return pointers;
        }
        
        template<size_t indexNum = 0>
        const char *getPointerAtIndex(uint32_t index) const {
            return dataFile.getDataAtOffset(getOffset<indexNum>(index));
        }
        
        template<size_t indexNum = 0>
        char *getPointerAtIndex(uint32_t index) {
            assert(index < size());
            auto offset = getOffset<indexNum>(index);
            return dataFile.getDataAtOffset(offset);
        }
        
        void write(const nth_element<0> &t) {
            writeNewImp(&t, sizeof(t));
        }
        
        void write(const ArbitraryLengthData<nth_element<0>> &t) {
            writeNewImp(t.dataView(), t.size());
        }
        
        template<size_t indexNum>
        void write(uint32_t addressNum, const nth_element<indexNum> &t) {
            writeUpdateImp<indexNum>(addressNum, &t, sizeof(t));
        }
        
        template<size_t indexNum>
        void write(uint32_t addressNum, const ArbitraryLengthData<nth_element<indexNum>> &t) {
            writeUpdateImp<indexNum>(addressNum, t.dataView(), t.size());
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
