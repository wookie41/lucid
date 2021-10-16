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
                InternalSaveToResourceFile(EFileFormat::Json);
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
            assert(AssetPath.GetLength());
            InternalSaveToResourceFile(InFileFormat);
        }
        else
        {
            bIsAsset = true;
            AssetId = sole::uuid4();
            Name = SPrintf("%s_%s", *Name, AssetId.str().c_str());
            AssetPath = SPrintf("assets/materials/%s.asset", *Name);
            GEngine.GetMaterialDatabase().Entries.push_back({AssetId, AssetPath, InFileFormat, GetType(), false});
            GEngine.SaveMaterialDatabase();
            SaveToResourceFile(InFileFormat);
        }
    }
}
