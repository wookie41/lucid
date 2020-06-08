#include "common/collections.hpp"
#include "common/bytes.hpp"

#include "stdlib.h"

#include <cassert>

namespace lucid
{
    template <typename T>
    StaticArray<T>::StaticArray(const uint32_t& ArrayCapacity)
    {
        Capacity = ArrayCapacity;
        SizeInBytes = sizeof(T) * ArrayCapacity; 
        array = (T*)malloc(SizeInBytes);
        zero(array, SizeInBytes);
    }

    template <typename T>
    T* StaticArray<T>::operator[](const uint32_t& Index) const
    {
        assert(Index < Length);
        return array + Index;
    }

    template <typename T>
    void StaticArray<T>::Add(const T& Element)
    {
        assert(Length < Capacity);
        array[Length++] = Element;
    }

    template <typename T>
    void StaticArray<T>::Free()
    {
        free(array);
        Capacity = -1;
        Length = -1;
    }

    template <typename T>
    StaticArray<T>::operator T*() const
    {
        return array;
    }

    template <typename T>
    StaticArray<T>::operator void*() const
    {
        return array;
    }
} // namespace lucid
