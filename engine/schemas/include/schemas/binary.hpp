#pragma once

// Binary Reading
#ifdef LUCID_SCHEMAS_IMPLEMENTATION
    #include "MakeBinaryReadHeader.h"
    #include "MakeBinaryReadFooter.h"
#else
    #include "MakeBinaryReadHeaderFWD.h"
    #include "MakeBinaryReadFooterFWD.h"
#endif
#include "schemas/schemas.hpp"

#ifdef LUCID_SCHEMAS_IMPLEMENTATION
    #include "MakeBinaryWriteHeader.h"
    #include "MakeBinaryWriteFooter.h"
#else
    #include "MakeBinaryWriteHeaderFWD.h"
    #include "MakeBinaryWriteFooterFWD.h"
#endif
#include "schemas/schemas.hpp"
