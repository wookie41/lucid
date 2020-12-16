#pragma once

#include <cstdint>

namespace lucid
{
    template <typename T>
    struct StaticArray
    {
        StaticArray(const uint32_t& Capacity);

        T* operator[](const uint32_t& Index) const;
        T* operator[](const int32_t& Index) const;
        operator T*() const;
        operator void*() const;

        StaticArray<T> Copy() const;
        void Add(const T& Element);
        void Free();
        void Resize(const uint32_t& NewCapacity);

        uint32_t Length = 0;
        uint32_t Capacity;
        uint64_t SizeInBytes;

      private:
        T* array;
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

        ~LinkedList();

        LinkedListItem<T> Head;
        LinkedListItem<T>* Tail;
    };
} // namespace lucid

#include "common/collections.tpp"