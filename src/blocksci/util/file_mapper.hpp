//
//  file_mapper.hpp
//  blocksci
//
//  Created by Harry Kalodner on 7/23/17.
//
//

#ifndef file_mapper_hpp
#define file_mapper_hpp

#include <blocksci/util/file_mapper_fwd.hpp>

#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/filesystem/path.hpp>

#include <range/v3/view_facade.hpp>
#include <range/v3/utility/optional.hpp>

#include <array>
#include <vector>

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
        
        void openFile(size_t size);
        
    protected:
        FileType file;
        size_t fileEnd;
    public:
        boost::filesystem::path path;
        AccessMode fileMode;
        
        void reload();
        
        SimpleFileMapperBase(boost::filesystem::path path_, AccessMode mode);
        
        bool isGood() const {
            return file.is_open();
        }
        
        void clearBuffer() {}
        
        const char *getDataAtOffset(OffsetType offset) const;
        
        size_t size() const {
            return fileEnd;
        }
        
        size_t fileSize() const;
    };
    
    template <>
    struct SimpleFileMapper<AccessMode::readonly> : public SimpleFileMapperBase {
        SimpleFileMapper(boost::filesystem::path path) : SimpleFileMapperBase(path, AccessMode::readonly) {}
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
    struct SimpleFileMapper<AccessMode::readwrite> : public SimpleFileMapperBase {
        static constexpr size_t maxBufferSize = 50000000;
        std::vector<char> buffer;
        static constexpr auto mode = AccessMode::readwrite;
        
        SimpleFileMapper(boost::filesystem::path path) : SimpleFileMapperBase(path, AccessMode::readwrite), writePos(size()) {}
        
        ~SimpleFileMapper() {
            clearBuffer();
        }
        
    private:
        OffsetType writePos;
        
        char *getWritePos();
        
        size_t writeSpace() const;
        
    public:
        
        void reload() {
            clearBuffer();
            SimpleFileMapperBase::reload();
        }
        
        OffsetType getWriteOffset() const {
            return writePos;
        }
        
        bool write(const char *valuePos, size_t amountToWrite);
        
        template<typename T, typename = std::enable_if_t<std::is_trivially_copyable<T>::value>>
        bool write(const T &t) {
            return write(reinterpret_cast<const char *>(&t), sizeof(t));
        }
        
        template<typename T>
        bool write(const ArbitraryLengthData<T> &t) {
            return write(t.dataView(), t.size());
        }
        
        void clearBuffer();
        
        char *getDataAtOffset(OffsetType offset);
        
        const char *getDataAtOffset(OffsetType offset) const;
        
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
    struct FixedSizeFileMapper  : public ranges::view_facade<FixedSizeFileMapper<T, mode>> {
    private:
        SimpleFileMapper<mode> dataFile;
        size_t getPos(size_t index) const {
            return index * sizeof(T);
        }
        
        friend ranges::range_access;
        
        struct cursor {
        private:
            const FixedSizeFileMapper<T, mode> *file;
            size_t currentIndex;
        public:
            cursor() = default;
            cursor(const FixedSizeFileMapper<T, mode> &file_, uint32_t txNum) : file(&file_), currentIndex(txNum) {}
            
            add_const_ptr_t<T> read() const {
                return file->getData(currentIndex);
            }
            
            bool equal(cursor const &that) const {
                return currentIndex == that.currentIndex;
            }
            
            bool equal(ranges::default_sentinel) const {
                return currentIndex == file->size();
            }
            
            void next() {
                currentIndex++;
            }
            
            int distance_to(cursor const &that) const {
                return static_cast<int>(currentIndex) - static_cast<int>(that.currentIndex);
            }
            
            int distance_to(ranges::default_sentinel) const {
                return static_cast<int>(file->size()) - static_cast<int>(currentIndex);
            }
            
            void prev() {
                currentIndex--;
            }
            
            void advance(int amount) {
                currentIndex += static_cast<size_t>(amount);
            }
        };
        
        cursor begin_cursor() const {
            return cursor(*this, 0);
        }
        
        ranges::default_sentinel end_cursor() const {
            return {};
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

        add_const_ptr_t<T> getDataAtIndex(size_t index) const {
            return getData(index);
        }
        
        add_ptr_t<T> getDataAtIndex(size_t index) {
            return getData(index);
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
            constexpr size_t unitSize = sizeof(T);
            return dataFile.size() / unitSize;
        }
        
        size_t fileSize() const {
            return dataFile.fileSize() / sizeof(T);
        }
        
        void truncate(size_t index) {
            dataFile.truncate(getPos(index));
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
        ranges::optional<uint32_t> find(Test test) const {
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
            return ranges::nullopt;
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
    struct IndexedFileMapper  : public ranges::view_facade<IndexedFileMapper<mode, T...>> {
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
        
        template<size_t indexNum = 0>
        OffsetType getOffset(uint32_t index) const {
            static_assert(indexNum < sizeof...(T), "Trying to fetch index out of bounds");
            auto indexData = indexFile.getData(index);
            auto offset = (*indexData)[indexNum];
            assert(offset < dataFile.size() || offset == InvalidFileIndex);
            return offset;
        }
        
        friend ranges::range_access;
        
        struct cursor {
        private:
            const IndexedFileMapper<mode, T...> *file;
            size_t currentIndex;
        public:
            cursor() = default;
            cursor(const IndexedFileMapper<mode, T...> &file_, uint32_t txNum) : file(&file_), currentIndex(txNum) {}
            
            auto read() const {
                return file->getData(currentIndex);
            }
            
            bool equal(cursor const &that) const {
                return currentIndex == that.currentIndex;
            }
            
            bool equal(ranges::default_sentinel) const {
                return currentIndex == file->size();
            }
            
            void next() {
                currentIndex++;
            }
            
            int distance_to(cursor const &that) const {
                return static_cast<int>(currentIndex) - static_cast<int>(that.currentIndex);
            }
            
            int distance_to(ranges::default_sentinel) const {
                return static_cast<int>(file->size()) - static_cast<int>(currentIndex);
            }
            
            void prev() {
                currentIndex--;
            }
            
            void advance(int amount) {
                currentIndex += static_cast<size_t>(amount);
            }
        };
        
        cursor begin_cursor() const {
            return cursor(*this, 0);
        }
        
        ranges::default_sentinel end_cursor() const {
            return {};
        }
        
    public:
        IndexedFileMapper(const boost::filesystem::path &pathPrefix) : dataFile(boost::filesystem::path(pathPrefix).concat("_data")), indexFile(boost::filesystem::path(pathPrefix).concat("_index")) {
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
            return *indexFile.getData(index);
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
        auto getData(std::enable_if_t<(Z <= 1), uint32_t> index) const {
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
    };
}

#endif /* file_mapper_hpp */
