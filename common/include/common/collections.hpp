#pragma once

#include <cstdint>

namespace lucid
{
    template <typename T>
    struct StaticArray
    {
        StaticArray(const uint32_t& Capacity);

        T* operator[](const uint32_t& Index) const;
        operator T*() const;
        operator void*() const;

        void Add(const T& Element);
        void Free();

        uint32_t Length = 0;
        uint32_t Capacity;
        uint64_t SizeInBytes;
        
      private:
        T* array;
    };

} // namespace lucid


#include "common/collections.tpp"