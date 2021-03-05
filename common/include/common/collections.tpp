#include "common/collections.hpp"
#include "common/bytes.hpp"

#include <string.h>
#include <stdlib.h>
#include <cassert>

namespace lucid
{
    template <typename T>
    Array<T>::Array(const u32& InCapacity, const bool& InAutoResize, const u8& InResizeFactor)
    {
        Capacity = InCapacity;
        AutoResize = InAutoResize;
        ResizeFactor = InResizeFactor;
        ArrayPointer = (T*)malloc(sizeof(T) * Capacity);
    }

    template <typename T>
    T* Array<T>::operator[](const u32& InIndex) const
    {
        assert(InIndex < Length);
        return ArrayPointer + InIndex;
    }
    
    template <typename T>
    void Array<T>::Add(const T& Element)
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
    void Array<T>::Free()
    {
        free(ArrayPointer);
        Capacity = -1;
        Length = -1;
    }

    template <typename T>
    Array<T>::operator void*() const
    {
        return ArrayPointer;
    }

    template <typename T>
    Array<T>& Array<T>::operator=(const Array& Rhs)
    {
        Length = Rhs.Length;
        Capacity = Rhs.Capacity;
        ArrayPointer = Rhs.ArrayPointer;
        return *this;
    }

    template <typename T>
    Array<T> Array<T>::Copy() const
    {
        Array<T> NewArray  { Length, AutoResize, ResizeFactor };
        memcpy(NewArray.ArrayPointer, ArrayPointer, GetSizeInBytes());
        NewArray.Length = Length;
        return NewArray;
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
