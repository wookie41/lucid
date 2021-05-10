#include "scene/material.hpp"

#include <engine/engine.hpp>

#include "schemas/types.hpp"

namespace lucid::scene
{
    void CMaterial::SaveToResourceFile(const EFileFormat& InFileFormat)
    {
        if (bIsAsset)
        {
            assert(ResourcePath.GetLength());
            InternalSaveToResourceFile(InFileFormat);
        }
        else
        {
            bIsAsset = true;
            ID = sole::uuid4();
            Name = SPrintf("%s_%s", *Name, ID.str().c_str());
            ResourcePath = SPrintf("assets/materials/%s.asset", *Name);
            GEngine.GetMaterialDatabase().Entries.push_back({ID, ResourcePath, InFileFormat, GetType(), false});
            GEngine.SaveMaterialDatabase();
            SaveToResourceFile(InFileFormat);
        }
    }
}
