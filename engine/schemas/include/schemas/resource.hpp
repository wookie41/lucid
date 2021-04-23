#include "resources/resource.hpp"

STRUCT_BEGIN(lucid, FResourceDatabaseEntry, "Mapping of resource name to asset path used in the loading process process ")
    STRUCT_FIELD(lucid::FDString, Name, "", "Name of the resource")
    STRUCT_FIELD(lucid::FDString, Path, "", "Path to the asset resource")
    STRUCT_FIELD(lucid::resources::EResourceType, Type, lucid::resources::TEXTURE, "Type of the resource")
STRUCT_END()

STRUCT_BEGIN(lucid, FResourceDatabase, "")
    STRUCT_DYNAMIC_ARRAY(FResourceDatabaseEntry, Entries, {}, "Array of the resource entries")
STRUCT_END()