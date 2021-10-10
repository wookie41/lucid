// Generates code to read binary data from memory and files

#include <cstdlib>
#include <cstring>
#include <common/strings.hpp>


#include "_common.h"

// Enums

#define ENUM_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) \
    bool BinaryRead(_NAMESPACE::_NAME& value, const TDYNAMICARRAY<char>& data, size_t& offset) \
    { \
        typedef _NAMESPACE::_NAME EnumType; \
        TSTRING stringValue; \
        if(!BinaryRead(stringValue, data, offset)) \
        { \
            DFS_LOG("Trying to read a " #_NAMESPACE "::" #_NAME " as a string, but we couldn't\n"); \
            return false; \
        }

#define ENUM_ITEM(_NAME, _DESCRIPTION) \
        if (!_stricmp(&stringValue[0], #_NAME)) \
        { \
            value = EnumType::_NAME; \
            return true; \
        }

#define ENUM_END() \
        DFS_LOG("Unknown Enum Value: \"%s\"", &stringValue[0]); \
        return false; \
    }

// Structs

#define STRUCT_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) \
    bool BinaryRead(_NAMESPACE::_NAME& value, const TDYNAMICARRAY<char>& data, size_t& offset) \
    {

#define STRUCT_INHERIT_BEGIN(_NAMESPACE, _NAME, _BASE, _DESCRIPTION) \
    bool BinaryRead(_NAMESPACE::_NAME& value, const TDYNAMICARRAY<char>& data, size_t& offset) \
    { \
        if (!BinaryRead(*(_BASE*)&value, data, offset)) \
            return false;

#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
        if (!BinaryRead(value.##_NAME, data, offset)) \
        { \
            DFS_LOG("Could not read member " #_NAME "\n"); \
            return false; \
        }

#define STRUCT_FIELD_NO_SERIALIZE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION)

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION) \
        { \
            int arrayCount = 0; \
            if (!BinaryRead(arrayCount, data, offset)) \
            { \
                DFS_LOG("Could not read array count of array " #_NAME "\n"); \
                return false; \
            } \
            TDYNAMICARRAY_RESIZE(value._NAME,arrayCount); \
            for (size_t index = 0; index < TDYNAMICARRAY_SIZE(value._NAME); ++index) \
            { \
                if(!BinaryRead(value._NAME[index], data, offset)) \
                { \
                    DFS_LOG("Could not read an array item for array " #_NAME "\n"); \
                    return false; \
                } \
            } \
        }

#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION) \
        { \
            for (size_t index = 0; index < _SIZE; ++index) \
            { \
                if(!BinaryRead(value._NAME[index], data, offset)) \
                { \
                    DFS_LOG("Could not read an array item for array " #_NAME "\n"); \
                    return false; \
                } \
            } \
        }

#define STRUCT_END() \
        return true; \
    }

// Variants

#define VARIANT_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) \
    bool BinaryRead(_NAMESPACE::_NAME& value, const TDYNAMICARRAY<char>& data, size_t& offset) \
    { \
        typedef _NAMESPACE::_NAME ThisType; \
        if(!BinaryRead(value._index, data, offset)) \
            return false;

#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION) \
        if (value._index == ThisType::c_index_##_NAME && !BinaryRead(value._NAME, data, offset)) \
            return false;

#define VARIANT_END() \
        return true; \
    }

// A catch all template type to make compile errors about unsupported types easier to understand

template <typename T>
bool BinaryRead(T& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    static_assert(false, __FUNCSIG__ ": Unsupported type encountered!");
    return false;
}

// Built in types

bool BinaryRead(uint8_t& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    if (offset + sizeof(value) > TDYNAMICARRAY_SIZE(data))
        return false;

    value = *(decltype(&value))&data[offset];
    offset += sizeof(value);

    return true;
}

bool BinaryRead(uint16_t& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    if (offset + sizeof(value) > TDYNAMICARRAY_SIZE(data))
        return false;

    value = *(decltype(&value))&data[offset];
    offset += sizeof(value);

    return true;
}

bool BinaryRead(uint32_t& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    if (offset + sizeof(value) > TDYNAMICARRAY_SIZE(data))
        return false;

    value = *(decltype(&value))&data[offset];
    offset += sizeof(value);

    return true;
}

bool BinaryRead(uint64_t& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    if (offset + sizeof(value) > TDYNAMICARRAY_SIZE(data))
        return false;

    value = *(decltype(&value))&data[offset];
    offset += sizeof(value);

    return true;
}

bool BinaryRead(int8_t& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    if (offset + sizeof(value) > TDYNAMICARRAY_SIZE(data))
        return false;

    value = *(decltype(&value))&data[offset];
    offset += sizeof(value);

    return true;
}

bool BinaryRead(int16_t& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    if (offset + sizeof(value) > TDYNAMICARRAY_SIZE(data))
        return false;

    value = *(decltype(&value))&data[offset];
    offset += sizeof(value);

    return true;
}

bool BinaryRead(int32_t& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    if (offset + sizeof(value) > TDYNAMICARRAY_SIZE(data))
        return false;

    value = *(decltype(&value))&data[offset];
    offset += sizeof(value);

    return true;
}

bool BinaryRead(int64_t& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    if (offset + sizeof(value) > TDYNAMICARRAY_SIZE(data))
        return false;

    value = *(decltype(&value))&data[offset];
    offset += sizeof(value);

    return true;
}

bool BinaryRead(float& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    if (offset + sizeof(value) > TDYNAMICARRAY_SIZE(data))
        return false;

    value = *(float*)&data[offset];
    offset += sizeof(value);

    return true;
}

bool BinaryRead(bool& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    int intValue = 0;
    if (!BinaryRead(intValue, data, offset))
        return false;

    value = intValue ? true : false;

    return true;
}

bool BinaryRead(lucid::FBinaryData& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    if (!BinaryRead(value.Size, data, offset))
        return false;
    offset += 1;

    value.Pointer = (char*)malloc(value.Size);
    memcpy(value.Pointer, &data[offset], value.Size);
    offset += value.Size;
    return true;
}

bool BinaryRead(TSTRING& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    // Yes, the strings are null terminated in the binary file
    value = (const char*)&data[offset];
    offset += strlen(&value[0]) + 1;
    return true;
}

bool BinaryRead(lucid::gpu::ETexturePixelFormat& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    int8_t int_value;
    if(BinaryRead(int_value, data, offset))
    {
        value = static_cast<lucid::gpu::ETexturePixelFormat>(int_value);
        return true;
    }
    return false;
}

bool BinaryRead(lucid::gpu::ETextureDataType& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    int8_t int_value;
    if(BinaryRead(int_value, data, offset))
    {
        value = static_cast<lucid::gpu::ETextureDataType>(int_value);
        return true;
    }
    return false;
}

bool BinaryRead(lucid::resources::EResourceType& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    int8_t int_value;
    if(BinaryRead(int_value, data, offset))
    {
        value = static_cast<lucid::resources::EResourceType>(int_value);
        return true;
    }
    return false;
}

bool BinaryRead(lucid::FDString& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    // Yes, the strings are null terminated in the binary file
    value = lucid::CopyToString((const char*)&data[offset]);
    offset += value.GetLength() + 1;
    return true;
}


bool BinaryRead(lucid::gpu::ETextureDataFormat& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    int8_t int_value;
    if(BinaryRead(int_value, data, offset))
    {
        value = static_cast<lucid::gpu::ETextureDataFormat>(int_value);
        return true;
    }
    return false;
}

bool BinaryRead(lucid::UUID& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    TSTRING string_value;
    if(BinaryRead(string_value, data, offset))
    {
        value = sole::rebuild(string_value);
        return true;
    }
    return false;
}

bool BinaryRead(lucid::scene::EStaticMeshType& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    int8_t int_value;
    if(BinaryRead(int_value, data, offset))
    {
        value = static_cast<lucid::scene::EStaticMeshType>(int_value);
        return true;
    }
    return false;
}

bool BinaryRead(lucid::scene::EActorType& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    int8_t int_value;
    if(BinaryRead(int_value, data, offset))
    {
        value = static_cast<lucid::scene::EActorType>(int_value);
        return true;
    }
    return false;
}

template <typename  T>
bool BinaryRead(lucid::InstancedVariable<T>& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    if(BinaryRead(value.bChanged, data, offset))
    {
        if (value.bChanged)
        {
            if (BinaryRead(value.Value, data, offset))
            {
                return true;
            }
            return false;
        }
        else
        {
            return true;
        }
    }
    return false;
}

bool BinaryRead(lucid::scene::EMaterialType& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    int8_t int_value;
    if(BinaryRead(int_value, data, offset))
    {
        value = static_cast<lucid::scene::EMaterialType>(int_value);
        return true;
    }
    return false;
}

bool BinaryRead(lucid::scene::ELightUnit& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    int8_t int_value;
    if(BinaryRead(int_value, data, offset))
    {
        value = static_cast<lucid::scene::ELightUnit>(int_value);
        return true;
    }
    return false;
}

bool BinaryRead(lucid::scene::ELightSourceType& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    int8_t int_value;
    if(BinaryRead(int_value, data, offset))
    {
        value = static_cast<lucid::scene::ELightSourceType>(int_value);
        return true;
    }
    return false;
}

bool BinaryRead(glm::vec3& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    if(!BinaryRead(value.x, data, offset))
    {
        return false;
    }
    if(!BinaryRead(value.y, data, offset))
    {
        return false;
    }
    if(!BinaryRead(value.z, data, offset))
    {
        return false;
    }
    return true;
}
