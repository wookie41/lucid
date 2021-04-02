#pragma once

#include "document.h"
#include "error/en.h"
#include "writer.h"
#include "prettywriter.h"
#include "stringbuffer.h"

// Reading JSON
#include "MakeJSONReadHeader.h"
#include "schemas/resources.hpp"
#include "schemas/scene.hpp"
#include "MakeJSONReadFooter.h"

// Equality testing (used by json writing to not write default values)
#include "MakeEqualityTests.h"
#include "schemas/resources.hpp"
#include "schemas/scene.hpp"

// Writing JSON
#include "MakeJSONWriteHeader.h"
#include "schemas/resources.hpp"
#include "schemas/scene.hpp"
#include "MakeJSONWriteFooter.h"