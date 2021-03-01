#pragma once

#include "common/types.hpp"

namespace lucid
{
    template <typename T>
    struct StaticArray
    {
        StaticArray(const u32& Capacity);

        T* operator[](const u32& Index) const;
        operator T*() const;
        operator void*() const;
        StaticArray<T>& operator=(const StaticArray& Rhs);

        StaticArray<T> Copy() const;
        void Add(const T& Element);
        void Free();
        void Resize(const u32& NewCapacity);

        u32 Length = 0;
        u32 Capacity;
        u64 SizeInBytes;

      private:
        T* Array;
    };

    template <typename T>
    struct LinkedListItem
    {
        T* Element = nullptr;
        LinkedListItem<T>* Next = nullptr;
        LinkedListItem<T>* Prev = nullptr;
    };

    template <typename T>
    struct LinkedList
    {
        LinkedList();

        void Add(T* Element);
        void Remove(T* Element);
        bool Contains(T* Element);
        void Free();

        LinkedListItem<T> Head;
        LinkedListItem<T>* Tail;
    };
} // namespace lucid

#include "common/collections.tpp"