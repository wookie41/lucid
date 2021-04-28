// Enums

#define ENUM_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) void BinaryWrite(const _NAMESPACE::_NAME& value, TDYNAMICARRAY<char>& output);
#define ENUM_ITEM(_NAME, _DESCRIPTION)
#define ENUM_END()
// Structs

#define STRUCT_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) void BinaryWrite(const _NAMESPACE::_NAME& value, TDYNAMICARRAY<char>& output);

#define STRUCT_INHERIT_BEGIN(_NAMESPACE, _NAME, _BASE, _DESCRIPTION) void BinaryWrite(const _NAMESPACE::_NAME& value, TDYNAMICARRAY<char>& output);
#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION)

#define STRUCT_FIELD_NO_SERIALIZE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION)

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION)

#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION)

#define STRUCT_END()

// Variants

#define VARIANT_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) void BinaryWrite(const _NAMESPACE::_NAME& value, TDYNAMICARRAY<char>& output)

#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION)

#define VARIANT_END() 