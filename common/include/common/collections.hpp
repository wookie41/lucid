#pragma once

#include "common/types.hpp"

namespace lucid
{
    template <typename T>
    struct Array
    {
        Array(const u32& InCapacity, const bool& InAutoResize = false, const u8& InResizeFactor = 2);

        T* operator[](const u32& InIndex) const;

        operator void*() const;
        
        Array<T>& operator=(const Array& Rhs);
        Array<T> Copy() const;

        void Add(const T& Element);
        void Free();

        inline u32 GetLength() const { return Length; }
        inline u32 GetCapacity() const { return Capacity; } 
        inline u64 GetSizeInBytes() const { return sizeof(T) * Length; } 

      private:
        T*      ArrayPointer;
        bool    AutoResize;
        u8      ResizeFactor;

        u32     Length = 0;
        u32     Capacity;
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