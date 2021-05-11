#pragma once

#include "common/collections.hpp"
#include "common/bytes.hpp"

#include <stdlib.h>
#include <cassert>

#include "stb_ds.h"

namespace lucid
{
    template <typename T>
    FArray<T>::FArray(const u32& InCapacity, const bool& InAutoResize, const u8& InResizeFactor)
    {
        Capacity = InCapacity;
        AutoResize = InAutoResize;
        ResizeFactor = InResizeFactor;
        ArrayPointer = (T*)malloc(sizeof(T) * Capacity);
    }

    template <typename T>
    T* FArray<T>::operator[](const u32& InIndex) const
    {
        assert(InIndex < Length);
        return ArrayPointer + InIndex;
    }
    
    template <typename T>
    void FArray<T>::Add(const T& Element)
    {
        if (Length == Capacity)
        {
            if (AutoResize)
            {
                u32 NewCapacity = Capacity * ResizeFactor;
                T* NewArray = (T*)CopyBytes((const char*)ArrayPointer, GetSizeInBytes(), sizeof(T) * NewCapacity);
                free(ArrayPointer);
                ArrayPointer = NewArray;
                Capacity = NewCapacity;
            }
            else    
            {
                assert(Length < Capacity);            
            }            
        }
        ArrayPointer[Length++] = Element;
    }

    template <typename T>
    void FArray<T>::RemoveLast()
    {
        if (Length)
        {
            --Length;
        }
    }
    
    template <typename T>
    void FArray<T>::Free()
    {
        free(ArrayPointer);
        Capacity = -1;
        Length = -1;
    }

    template <typename T>
    FArray<T>::operator void*() const
    {
        return ArrayPointer;
    }

    template <typename T>
    FArray<T>& FArray<T>::operator=(const FArray& Rhs)
    {
        Length = Rhs.Length;
        Capacity = Rhs.Capacity;
        ArrayPointer = Rhs.ArrayPointer;
        return *this;
    }

    template <typename T>
    FArray<T> FArray<T>::Copy() const
    {
        FArray<T> NewArray  { Length, AutoResize, ResizeFactor };
        memcpy(NewArray.ArrayPointer, ArrayPointer, GetSizeInBytes());
        NewArray.Length = Length;
        return NewArray;
    }

    template <typename T>
    FLinkedList<T>::FLinkedList()
    {
        Tail = &Head;
    }

    template <typename T>
    void FLinkedList<T>::Add(T* Element)
    {
        Tail->Element = Element;
        Tail->Next = new FLinkedListItem<T>;
        Tail->Next->Prev = Tail;
        Tail = Tail->Next;
    }

    template <typename T>
    void FLinkedList<T>::Remove(T* Element)
    {
        FLinkedListItem<T>* current = &Head;
        while (current != nullptr)
        {
            if (current->Element == Element)
            {
                if (current->Prev)
                {
                    current->Prev->Next = current->Next;
                }
                if (current->Next)
                {
                    current->Next->Prev = current->Prev;                    
                }
                if (current == &Head)
                {
                    current->Element = nullptr;
                    current->Next = nullptr;
                    Tail = &Head;
                }
                else
                {
                    delete current;
                }
                return;
            }
            current = current->Next;
        }
    }

    template <typename T>
    bool FLinkedList<T>::Contains(T* Element)
    {
        FLinkedListItem<T>* current = &Head;
        while (current != nullptr)
        {
            if (current->Element == Element)
                return true;
        }
        return false;
    }

    template <typename T>
    void FLinkedList<T>::Free()
    {
        FLinkedListItem<T>* tmp = nullptr;
        FLinkedListItem<T>* current = Head.Next;
        while (current != nullptr)
        {
            tmp = current->Next;
            delete current;
            current = tmp;
        }
    }

    template <typename K, typename V>
    void FHashMap<K, V>::Add(const K& Key, const V& Value)
    {
        hmput(HashMap, Key, Value);
    }
    
    template <typename K, typename V>
    V& FHashMap<K, V>::Get(const K& Key)
    {
        return hmget(HashMap, Key);
    }

    template <typename K, typename V>
    bool FHashMap<K, V>::Contains(const K& Key)
    {
        return hmgeti(HashMap, Key) != -1;
    }

    template <typename K, typename V>
    u32 FHashMap<K, V>::GetLength() const
    {
        return hmlen(HashMap);
    }   

    template <typename K, typename V>
    V& FHashMap<K, V>::GetByIndex(const u64& EntryNum)
    {
        assert(EntryNum < GetLength());
        return HashMap[EntryNum].value;
    }

    template <typename K, typename V>
    void FHashMap<K, V>::FreeAll()
    {
        hmfree(HashMap);
    }

    template <typename K, typename V>
    void FHashMap<K, V>::Remove(const K& Key)
    {
        hmdel(HashMap, Key);
    }

    template <typename V>
    void FStringHashMap<V>::Add(const char* Key, const V& Value)
    {
        shput(HashMap, Key, Value);
    }
    
    template <typename V>
    V& FStringHashMap<V>::Get(const char* Key)
    {
        return shget(HashMap, Key);
    }

    template <typename V>
    bool FStringHashMap<V>::Contains(const char* Key)
    {
        return shgeti(HashMap, Key) != -1;
    }

    template <typename V>
    u32 FStringHashMap<V>::GetLength() const
    {
        return hmlen(HashMap);
    }   

    template <typename V>
    V& FStringHashMap<V>::Get(const u64& EntryNum) const
    {
        assert(EntryNum < GetLength());
        return HashMap[EntryNum].value;
    }

} // namespace lucid
