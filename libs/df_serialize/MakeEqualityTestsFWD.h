// This makes member wise equality testing

#pragma once

#include "_common.h"

// Enums - they already have a == operator built in

#define ENUM_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION)

#define ENUM_ITEM(_NAME, _DESCRIPTION)

#define ENUM_END()

// Structs

#define STRUCT_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) \
    bool operator == (const _NAMESPACE::_NAME& A, const _NAMESPACE::_NAME& B); \
    bool operator != (const _NAMESPACE::_NAME& A, const _NAMESPACE::_NAME& B); \
    bool operator == (const _NAMESPACE::_NAME& A, const _NAMESPACE::_NAME& B);

#define STRUCT_INHERIT_BEGIN(_NAMESPACE, _NAME, _BASE, _DESCRIPTION) \
    bool operator == (const _NAMESPACE::_NAME& A, const _NAMESPACE::_NAME& B); \
    bool operator != (const _NAMESPACE::_NAME& A, const _NAMESPACE::_NAME& B); \
    bool operator == (const _NAMESPACE::_NAME& A, const _NAMESPACE::_NAME& B);

#define STRUCT_FIELD(_TYPE, _NAME, _DEFAULT, _DESCRIPTION)

// No serialize also means no equality test
#define STRUCT_FIELD_NO_SERIALIZE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION)

#define STRUCT_DYNAMIC_ARRAY(_TYPE, _NAME, _DESCRIPTION)

#define STRUCT_STATIC_ARRAY(_TYPE, _NAME, _SIZE, _DEFAULT, _DESCRIPTION)
#define STRUCT_END()

// Variants

#define VARIANT_BEGIN(_NAMESPACE, _NAME, _DESCRIPTION) \
    bool operator == (const _NAMESPACE::_NAME& A, const _NAMESPACE::_NAME& B); \
    bool operator != (const _NAMESPACE::_NAME& A, const _NAMESPACE::_NAME& B); \
    bool operator == (const _NAMESPACE::_NAME& A, const _NAMESPACE::_NAME& B);

#define VARIANT_TYPE(_TYPE, _NAME, _DEFAULT, _DESCRIPTION)
#define VARIANT_END()
