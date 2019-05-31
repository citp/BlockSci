//
//  in_place_array.hpp
//  blocksci
//
//  Created by Harry Kalodner on 4/10/18.
//

#ifndef in_place_array_h
#define in_place_array_h

namespace blocksci {
    template <typename T, typename Index = uint32_t>
    class InPlaceArray {
        Index dataSize;
        
    public:
        explicit InPlaceArray(Index size) : dataSize(size) {}
        
        Index size() const {
            return dataSize;
        }
        
        T &operator[](Index index) {
            return *(reinterpret_cast<T *>(reinterpret_cast<char *>(this) + sizeof(InPlaceArray)) + index);
        }
        
        const T &operator[](Index index) const {
            return *(reinterpret_cast<const T *>(reinterpret_cast<const char *>(this) + sizeof(InPlaceArray)) + index);
        }
        
        const T *begin() const {
            return &operator[](Index{0});
        }
        
        const T *end() const {
            return &operator[](size());
        }
        
        const T *begin() {
            return &operator[](Index{0});
        }
        
        const T *end() {
            return &operator[](size());
        }
        
        size_t extraSize() const {
            return sizeof(T) * size();
        }
        
    };
} // namespace blocksci

#endif /* in_place_array_h */
