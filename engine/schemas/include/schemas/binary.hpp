#pragma once

// Binary Reading
#ifdef LUCID_SCHEMAS_IMPLEMENTATION
    #include "MakeBinaryReadHeader.h"
#else
    #include "MakeBinaryReadHeaderFWD.h"
#endif
#include "schemas/schemas.hpp"
#include "MakeBinaryReadFooter.h"

#ifdef LUCID_SCHEMAS_IMPLEMENTATION
    #include "MakeBinaryWriteHeader.h"
#else
    #include "MakeBinaryWriteHeaderFWD.h"
#endif
#include "schemas/schemas.hpp"
#include "MakeBinaryWriteFooter.h"
