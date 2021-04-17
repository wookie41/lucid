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
#include "MakeEqualityTests.h"
#include "schemas/schemas.hpp"

// Writing JSON
#include "MakeJSONWriteHeader.h"
#include "schemas/schemas.hpp"
#include "MakeJSONWriteFooter.h"