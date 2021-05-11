#include "scene/material.hpp"
#include "engine/engine.hpp"
#include "schemas/types.hpp"
#include "imgui.h"

namespace lucid::scene
{
#if DEVELOPMENT
    void CMaterial::UIDrawMaterialEditor()
    {
        static char RenameBuffer[256];

        if (bIsRenaming)
        {
            ImGui::InputText("##Rename Material", RenameBuffer, 255);
            ImGui::SameLine();
            if (ImGui::Button("Rename"))
            {
                Name.ReplaceWithBuffer(RenameBuffer);
                bIsRenaming = false;
            }
        }
        else
        {
            ImGui::Text("Material name: %s", *Name);
            ImGui::SameLine();
            if (ImGui::Button("Rename"))
            {
                Name.CopyToBuffer(RenameBuffer);
                bIsRenaming = true;
            }
        }
    }
#endif

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
