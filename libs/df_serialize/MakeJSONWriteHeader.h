// Generates code to write data to json files.

#include <resources/resource.hpp>

#include "_common.h"
#include "common/bytes.hpp"

// Enums

#define ENUM_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) rapidjson::Value MakeJSONValue(const _NAMESPACE::_NAME& value, rapidjson::Document::AllocatorType& allocator);

#define ENUM_ITEM(_NAME, _DESCRIPTION)
#define ENUM_END()

// Structs

#define STRUCT_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) rapidjson::Value MakeJSONValue(const _NAMESPACE::_NAME& value, rapidjson::Document::AllocatorType& allocator);

#define STRUCT_INHERIT_BEGIN(_NAMESPACE, _NAME, _BASE, _DESCRIPTION) rapidjson::Value MakeJSONValue(const _NAMESPACE::_NAME& value, rapidjson::Document::AllocatorType& allocator);

#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION)
#define STRUCT_FIELD_NO_SERIALIZE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION)

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION)
#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION)
#define STRUCT_END()

// Variants

#define VARIANT_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) rapidjson::Value MakeJSONValue(const _NAMESPACE::_NAME& value, rapidjson::Document::AllocatorType& allocator);

// NOTE: always save out the variant object, so we know what _index is!
#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION)
#define VARIANT_END()