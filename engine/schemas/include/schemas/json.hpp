#pragma once

#include "document.h"
#include "error/en.h"
#include "writer.h"
#include "prettywriter.h"
#include "stringbuffer.h"

// Reading JSON
#include "MakeJSONReadHeader.h"
#include "schemas/schemas.hpp"
#include "MakeJSONReadFooter.h"

// Equality testing (used by json writing to not write default values)
#ifdef LUCID_SCHEMAS_IMPLEMENTATION
    #include "MakeEqualityTests.h"
#else
    #include "MakeEqualityTestsFWD.h"
#endif
#include "schemas/schemas.hpp"

// Writing JSON
#ifdef LUCID_SCHEMAS_IMPLEMENTATION
    #include "MakeJSONWriteHeader.h"
#else
    #include "MakeJSONWriteHeaderFWD.h"
#endif
#include "schemas/schemas.hpp"
#include "MakeJSONWriteFooter.h"
