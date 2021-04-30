// Generates code to read binary data from memory and files

#include <vector>
#include <array>
#include <string>

#include "common/bytes.hpp"
#include "_common.h"

// Enums

#define ENUM_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) bool BinaryRead(_NAMESPACE::_NAME& value, const TDYNAMICARRAY<char>& data, size_t& offset);

#define ENUM_ITEM(_NAME, _DESCRIPTION);
#define ENUM_END();

// Structs

#define STRUCT_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) BinaryRead(_NAMESPACE::_NAME& value, const TDYNAMICARRAY<char>& data, size_t& offset);

#define STRUCT_INHERIT_BEGIN(_NAMESPACE, _NAME, _BASE, _DESCRIPTION) bool BinaryRead(_NAMESPACE::_NAME& value, const TDYNAMICARRAY<char>& data, size_t& offset);

#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION);

#define STRUCT_FIELD_NO_SERIALIZE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION)

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION);
#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION);

#define STRUCT_END();
// Variants

#define VARIANT_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) bool BinaryRead(_NAMESPACE::_NAME& value, const TDYNAMICARRAY<char>& data, size_t& offset);

#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION);
#define VARIANT_END();

// A catch all template type to make compile errors about unsupported types easier to understand

template <typename T>
bool BinaryRead(T& value, const TDYNAMICARRAY<char>& data, size_t& offset)
{
    static_assert(false, __FUNCSIG__ ": Unsupported type encountered!");
    return false;
}

// Built in types

bool BinaryRead(uint8_t& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(uint16_t& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(uint32_t& value, const TDYNAMICARRAY<char>& data, size_t& offset);

bool BinaryRead(uint64_t& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(int8_t& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(int16_t& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(int32_t& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(int64_t& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(float& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(bool& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(lucid::FBinaryData& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(TSTRING& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(lucid::gpu::ETexturePixelFormat& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(lucid::gpu::ETextureDataType& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(lucid::resources::EResourceType& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(lucid::FDString& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(lucid::gpu::ETextureDataFormat& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(lucid::UUID& value, const TDYNAMICARRAY<char>& data, size_t& offset);
bool BinaryRead(lucid::scene::EStaticMeshType& value, const TDYNAMICARRAY<char>& data, size_t& offset);