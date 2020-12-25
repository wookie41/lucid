#include "common/collections.hpp"
#include "common/bytes.hpp"

#include <string.h>
#include <stdlib.h>
#include <cassert>

namespace lucid
{
    template <typename T>
    StaticArray<T>::StaticArray(const uint32_t& ArrayCapacity)
    {
        Capacity = ArrayCapacity;
        SizeInBytes = sizeof(T) * ArrayCapacity;
        array = (T*)malloc(SizeInBytes);
        Zero(array, SizeInBytes);
    }

    template <typename T>
    T* StaticArray<T>::operator[](const uint32_t& Index) const
    {
        assert(Index < Length);
        return array + Index;
    }

    template <typename T>
    T* StaticArray<T>::operator[](const int32_t& Index) const
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
    void StaticArray<T>::Resize(const uint32_t& NewCapacity)
    {
        int newArraySize = sizeof(T) * NewCapacity;
        T* newArray = (T*)malloc(newArraySize);
        Zero(newArray, newArraySize);
        memcpy(newArray, array, sizeof(T) * Length);
        free(array);
        array = newArray;
        Capacity = NewCapacity;
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

    template <typename T>
    StaticArray<T> StaticArray<T>::Copy() const
    {
        StaticArray<T> newArray(Length);
        memcpy(newArray.array, array, newArray.SizeInBytes);
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
