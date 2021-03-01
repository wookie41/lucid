#include "common/collections.hpp"
#include "common/bytes.hpp"

#include <string.h>
#include <stdlib.h>
#include <cassert>

namespace lucid
{
    template <typename T>
    StaticArray<T>::StaticArray(const u32& ArrayCapacity)
    {
        Capacity = ArrayCapacity;
        SizeInBytes = sizeof(T) * ArrayCapacity;
        Array = (T*)malloc(SizeInBytes);
        Zero(Array, SizeInBytes);
    }
    
    template <typename T>
    T* StaticArray<T>::operator[](const u32& Index) const
    {
        assert(Index < Length);
        return Array + Index;
    }

    
    template <typename T>
    void StaticArray<T>::Add(const T& Element)
    {
        assert(Length < Capacity);
        Array[Length++] = Element;
    }

    template <typename T>
    void StaticArray<T>::Free()
    {
        free(Array);
        Capacity = -1;
        Length = -1;
    }

    template <typename T>
    void StaticArray<T>::Resize(const u32& NewCapacity)
    {
        int newArraySize = sizeof(T) * NewCapacity;
        T* newArray = (T*)malloc(newArraySize);
        Zero(newArray, newArraySize);
        memcpy(newArray, Array, sizeof(T) * Length);
        free(Array);
        Array = newArray;
        Capacity = NewCapacity;
    }

    template <typename T>
    StaticArray<T>::operator T*() const
    {
        return Array;
    }

    template <typename T>
    StaticArray<T>::operator void*() const
    {
        return Array;
    }

    template <typename T>
    StaticArray<T>& StaticArray<T>::operator=(const StaticArray& Rhs)
    {
        Length = Rhs.Length;
        Capacity = Rhs.Capacity;
        SizeInBytes = Rhs.SizeInBytes;
        Array = Rhs.Array;
        return *this;
    }

    template <typename T>
    StaticArray<T> StaticArray<T>::Copy() const
    {
        StaticArray<T> newArray(Length);
        memcpy(newArray.Array, Array, newArray.SizeInBytes);
        newArray.Length = Length;
        return newArray;
    }

    template <typename T>
    LinkedList<T>::LinkedList()
    {
        Tail = &Head;
    }

    template <typename T>
    void LinkedList<T>::Add(T* Element)
    {
        Tail->Element = Element;
        Tail->Next = new LinkedListItem<T>;
        Tail->Next->Prev = Tail;
        Tail = Tail->Next;
    }

    template <typename T>
    void LinkedList<T>::Remove(T* Element)
    {
        LinkedListItem<T>* current = &Head;
        while (current != nullptr)
        {
            if (current->Element == Element)
            {
                current->Prev->Next = current->Next;
                current->Next->Prev = current->Prev;
                delete current;
                return;
            }
        }
    }

    template <typename T>
    bool LinkedList<T>::Contains(T* Element)
    {
        LinkedListItem<T>* current = &Head;
        while (current != nullptr)
        {
            if (current->Element == Element)
                return true;
        }
        return false;
    }

    template <typename T>
    void LinkedList<T>::Free()
    {
        LinkedListItem<T>* tmp = nullptr;
        LinkedListItem<T>* current = Head.Next;
        while (current != nullptr)
        {
            tmp = current->Next;
            delete current;
            current = tmp;
        }
    }

} // namespace lucid
