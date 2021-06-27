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
        void RemoveLast();
        void Free();

        inline u32 GetLength() const { return Length; }
        inline u32 GetCapacity() const { return Capacity; } 
        inline u64 GetSizeInBytes() const { return sizeof(T) * Length; }
        inline T* Last()
        {
            assert(Length);
            return ArrayPointer + (Length - 1);
        }

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
        bool IsEmpty() const;
        
        FLinkedListItem<T> Head;
        FLinkedListItem<T>* Tail;
    };

    template <typename K, typename V>
    struct FHashMapEntry
    {
        K   key;
        V   value;
    };

    template <typename K, typename V>
    struct FHashMap
    {
    public:

        void                    Add(const K& Key, const V& Value);
        V&                      Get(const K& Key);
        FHashMapEntry<K, V>&    GetEntryByIndex(const u64& EntryNum) const
        {
            assert(EntryNum < GetLength());
            return HashMap[EntryNum];
        }
        V&                      GetByIndex(const u64& EntryNum) const;
        void                    Remove(const K& Key);
        bool                    Contains(const K& Key);
        u32                     GetLength() const;
        void                    FreeAll();

    private:
        
        FHashMapEntry<K, V>* HashMap = NULL;
    };


    template <typename V>
    struct FStringHashMap
    {
    public:

        void        Add(const char* Key, const V& Value);
        V&          Get(const char* Key);
        V&          Get(const u64& EntryNum) const;
        bool        Contains(const char* Key);
        u32         GetLength() const;
    private:
        
        struct
        {
            char*   key;
            V       value;
        }* HashMap = NULL;
    };

    
} // namespace lucid

#include "common/collections.tpp"