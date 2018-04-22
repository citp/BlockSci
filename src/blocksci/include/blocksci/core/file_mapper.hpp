//
//  file_mapper.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/23/17.
//
//

#ifndef file_mapper_hpp
#define file_mapper_hpp

#include <blocksci/blocksci_export.h>

#include <boost/filesystem/path.hpp>

#include <array>
#include <vector>

namespace boost { namespace iostreams {
    class mapped_file;
}}

namespace blocksci {
    enum class AccessMode {
        readonly, readwrite
    };
    
    template<AccessMode mode = AccessMode::readonly>
    struct SimpleFileMapper;
    
    template <typename T, AccessMode mode = AccessMode::readonly>
    struct FixedSizeFileMapper;
    
    template <AccessMode mode = AccessMode::readonly, typename... T>
    struct IndexedFileMapper;
    
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

    struct BLOCKSCI_EXPORT SimpleFileMapperBase {
    private:
        void openFile(size_t size);
    protected:
        std::unique_ptr<boost::iostreams::mapped_file> file;
        size_t fileEnd;
        const char *const_data;
    public:
        boost::filesystem::path path;
        AccessMode fileMode;
        
        SimpleFileMapperBase(boost::filesystem::path path_, AccessMode mode);
        SimpleFileMapperBase(SimpleFileMapperBase &&other);
        ~SimpleFileMapperBase();
        
        bool isGood() const;
        
        void clearBuffer() {}
        
        const char *getDataAtOffset(OffsetType offset) const {
            if (offset == InvalidFileIndex) {
                return nullptr;
            }
            assert(offset < size());
            return const_data + offset;
        }
        
        size_t size() const {
            return fileEnd;
        }
        
        size_t fileSize() const;
        
        void reload();
    };
    
    template <>
    struct BLOCKSCI_EXPORT SimpleFileMapper<AccessMode::readonly> : public SimpleFileMapperBase {
        SimpleFileMapper(boost::filesystem::path path) : SimpleFileMapperBase(std::move(path), AccessMode::readonly) {}
    };
    
    template <typename MainType>
    class ArbitraryLengthData {
        std::vector<char> rawData;
        
    public:
        explicit ArbitraryLengthData(const MainType &head) {
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
        
        const char *finalize() {
            auto charsToAdd = rawData.size() % alignof(MainType);
            if (charsToAdd > 0) {
                for (size_t i = 0; i < (alignof(MainType) - charsToAdd); i++) {
                    rawData.push_back(0);
                }
            }
            return rawData.data();
        }
    };
    
    template <>
    struct BLOCKSCI_EXPORT SimpleFileMapper<AccessMode::readwrite> : public SimpleFileMapperBase {
        static constexpr size_t maxBufferSize = 50000000;
        std::vector<char> buffer;
        static constexpr auto mode = AccessMode::readwrite;
        
        explicit SimpleFileMapper(boost::filesystem::path path);
        
        SimpleFileMapper(const SimpleFileMapper &) = delete;
        SimpleFileMapper(SimpleFileMapper &&) = default;
        SimpleFileMapper &operator=(const SimpleFileMapper &) = delete;
        SimpleFileMapper &operator=(SimpleFileMapper &&) = default;

        ~SimpleFileMapper() {
            clearBuffer();
        }
        
    private:
        OffsetType writePos;
        char *dataPtr;
        
        char *getWritePos() {
            if (writePos < fileEnd) {
                return reinterpret_cast<char *>(dataPtr) + writePos;
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
        
        void reload();
        
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
        
        void clearBuffer();
        
        char *getDataAtOffset(OffsetType offset) {
            assert(offset < fileEnd + buffer.size() || offset == InvalidFileIndex);
            if (offset == InvalidFileIndex) {
                return nullptr;
            } else if (offset < fileEnd) {
                return dataPtr + offset;
            } else {
                return buffer.data() + (offset - fileEnd);
            }
        }
        
        const char *getDataAtOffset(OffsetType offset) const {
            assert(offset < fileEnd + buffer.size() || offset == InvalidFileIndex);
            if (offset == InvalidFileIndex) {
                return nullptr;
            } else if (offset < fileEnd) {
                return const_data + offset;
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
        
        void truncate(OffsetType offset);
    };
    
    template <typename T, AccessMode mode>
    struct BLOCKSCI_EXPORT FixedSizeFileMapper {
    private:
        SimpleFileMapper<mode> dataFile;
        size_t getPos(size_t index) const {
            return index * sizeof(T);
        }
        
    public:
        using size_type = uint64_t;
        using difference_type = std::ptrdiff_t;
        using pointer = add_ptr_t<T>;
        using const_pointer = add_const_ptr_t<T>;

        
        explicit FixedSizeFileMapper(boost::filesystem::path path) : dataFile(std::move(path)) {}
        
        const_pointer operator[](size_type index) const {
            assert(index < size());
            const char *pos = dataFile.getDataAtOffset(getPos(index));
            return reinterpret_cast<const_pointer>(pos);
        }
        
        template <typename Dummy = void, typename Dummy2 = std::enable_if_t<mode == AccessMode::readwrite, Dummy>>
        pointer operator[](size_type index) {
            assert(index < size());
            char *pos = dataFile.getDataAtOffset(getPos(index));
            return reinterpret_cast<pointer>(pos);
        }

        const_pointer getDataAtIndex(size_type index) const {
            return (*this)[index];
        }
        
        pointer getDataAtIndex(size_type index) {
            return (*this)[index];
        }

        void seekEnd() {
            dataFile.seekEnd();
        }
        
        void seek(size_type index) {
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
            constexpr size_t unitSize = sizeof(T);
            return dataFile.size() / unitSize;
        }
        
        size_t fileSize() const {
            return dataFile.fileSize() / sizeof(T);
        }
        
        void truncate(size_type index) {
            dataFile.truncate(getPos(index));
        }
        
        template<typename Test>
        std::vector<uint32_t> findAll(Test test) const {
            auto itemCount = size();
            std::vector<uint32_t> indexes;
            uint32_t index = 0;
            while (index < itemCount) {
                auto nextItem = *this(index);
                if (test(*nextItem)) {
                    indexes.push_back(index);
                }
                index++;
            }
            return indexes;
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
    
    template <typename ...A, size_t N>
    add_ptr_t<std::tuple<A...>> tuple_cast(std::array<char *, N> t1) {
        static_assert(sizeof...(A) == N, "The tuple sizes must be the same");
        return tuple_cast_helper<add_ptr_t<std::tuple<A...>>>( t1, std::make_index_sequence<N>{} );
    }
    
    template <AccessMode mode, typename... T>
    struct BLOCKSCI_EXPORT IndexedFileMapper {
    private:
        template<size_t indexNum>
        using nth_element = std::tuple_element_t<indexNum, std::tuple<T...>>;
        static constexpr size_t indexCount = sizeof...(T);
        SimpleFileMapper<mode> dataFile;
        FixedSizeFileMapper<FileIndex<indexCount>, mode> indexFile;
        
        void writeNewImp(const char *valuePos, size_t amountToWrite) {
            assert(amountToWrite % alignof(nth_element<0>) == 0);
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
            assert(amountToWrite % alignof(nth_element<indexNum>) == 0);
            auto &fileIndex = *indexFile[addressNum];
            fileIndex[indexNum] = dataFile.getWriteOffset();
            dataFile.write(valuePos, amountToWrite);
            
        }
        
        template<size_t indexNum = 0>
        OffsetType getOffset(uint32_t index) const {
            static_assert(indexNum < sizeof...(T), "Trying to fetch index out of bounds");
            auto indexData = indexFile[index];
            auto offset = (*indexData)[indexNum];
            assert(offset < dataFile.size() || offset == InvalidFileIndex);
            return offset;
        }
        
    public:
        explicit IndexedFileMapper(const boost::filesystem::path &pathPrefix) : dataFile(boost::filesystem::path(pathPrefix).concat("_data")), indexFile(boost::filesystem::path(pathPrefix).concat("_index")) {
        }
        
        void reload() {
            indexFile.reload();
            dataFile.reload();
        }
        
        void clearBuffer() {
            indexFile.clearBuffer();
            dataFile.clearBuffer();
        }
        
        FileIndex<sizeof...(T)> getOffsets(uint32_t index) const {
            return *indexFile[index];
        }
        
        size_t size() const {
            return indexFile.size();
        }
        
        size_t fileSize() const {
            return indexFile.fileSize();
        }
        
        void truncate(uint32_t index) {
            if (index < size()) {
                auto offsets = getOffsets(index);
                indexFile.truncate(index);
                dataFile.truncate(offsets[0]);
            }
        }
        
        void seekEnd() {
            indexFile.seekEnd();
            dataFile.seekEnd();
        }
        
        void seek(uint32_t index, size_t dataOffset) {
            indexFile.seek(index);
            dataFile.seek(dataOffset);
        }

        void grow(uint32_t indexSize, uint64_t dataSize) {
            indexFile.truncate(indexFile.size() + indexSize);
            dataFile.truncate(dataFile.size() + dataSize);
        }
        
        void write(const nth_element<0> &t) {
            writeNewImp(&t, sizeof(t));
        }
        
        void write(ArbitraryLengthData<nth_element<0>> &t) {
            auto finalData = t.finalize();
            writeNewImp(finalData, t.size());
        }
        
        template<size_t indexNum>
        void write(uint32_t addressNum, const nth_element<indexNum> &t) {
            writeUpdateImp<indexNum>(addressNum, &t, sizeof(t));
        }
        
        template<size_t indexNum>
        void write(uint32_t addressNum, ArbitraryLengthData<nth_element<indexNum>> &t) {
            auto finalData = t.finalize();
            writeUpdateImp<indexNum>(addressNum, finalData, t.size());
        }
        
        template<size_t indexNum = 0>
        add_const_ptr_t<nth_element<indexNum>> getDataAtIndex(uint32_t index) const {
            assert(index < size());
            auto offset = getOffset<indexNum>(index);
            auto pointer = dataFile.getDataAtOffset(offset);
            return reinterpret_cast<add_const_ptr_t<nth_element<indexNum>>>(pointer);
        }
        
        template<size_t indexNum = 0>
        add_ptr_t<nth_element<indexNum>> getDataAtIndex(uint32_t index) {
            assert(index < size());
            auto offset = getOffset<indexNum>(index);
            auto pointer = dataFile.getDataAtOffset(offset);
            return reinterpret_cast<add_ptr_t<nth_element<indexNum>>>(pointer);
        }
        
        template <size_t Z = sizeof...(T)>
        auto getData(std::enable_if_t<(Z > 1), uint32_t> index) const {
            assert(index < size());
            auto offsets = getOffsets(index);
            std::array<const char *, sizeof...(T)> pointers;
            for (size_t i = 0; i < sizeof...(T); i++) {
                pointers[i] = dataFile.getDataAtOffset(offsets[i]);
            }
            return tuple_cast<T...>(pointers);
        }
        
        template <size_t Z = sizeof...(T)>
        auto getData(std::enable_if_t<(Z == 1), uint32_t> index) const {
            assert(index < size());
            return getDataAtIndex<0>(index);
        }
        
        template <size_t Z = sizeof...(T)>
        auto getData(std::enable_if_t<(Z > 1), uint32_t> index) {
            assert(index < size());
            auto offsets = getOffsets(index);
            std::array<char *, sizeof...(T)> pointers;
            for (size_t i = 0; i < sizeof...(T); i++) {
                pointers[i] = dataFile.getDataAtOffset(offsets[i]);
            }
            return tuple_cast<T...>(pointers);
        }
        
        template <size_t Z = sizeof...(T)>
        auto getData(std::enable_if_t<(Z <= 1), uint32_t> index) {
            assert(index < size());
            return getDataAtIndex<0>(index);
        }

        auto operator[](uint32_t index) const {
            return getData(index);
        }

        auto operator[](uint32_t index) {
            return getData(index);
        }
    };
}

#endif /* file_mapper_hpp */
