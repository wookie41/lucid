#pragma once

#include "common/types.hpp"

namespace lucid
{
    template <typename T>
    struct FArray
    {
        FArray(const u32& InCapacity, const bool& InAutoResize = false, const u8& InResizeFactor = 2);

        T* operator[](const u32& InIndex) const;

        operator void*() const;
        
        FArray<T>& operator=(const FArray& Rhs);
        FArray<T> Copy() const;

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
    struct FLinkedListItem
    {
        T* Element = nullptr;
        FLinkedListItem<T>* Next = nullptr;
        FLinkedListItem<T>* Prev = nullptr;
    };

    template <typename T>
    struct FLinkedList
    {
        FLinkedList();

        void Add(T* Element);
        void Remove(T* Element);
        bool Contains(T* Element);
        void Free();

        FLinkedListItem<T> Head;
        FLinkedListItem<T>* Tail;
    };
} // namespace lucid

#include "common/collections.tpp"