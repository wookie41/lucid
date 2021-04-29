#include "scene/actors/static_mesh.hpp"

#include "engine/engine.hpp"
#include "imgui.h"

namespace lucid::scene
{
    CStaticMesh::CStaticMesh(const FDString& InName,
                             const IActor* InParent,
                             resources::CMeshResource* InMeshResource,
                             CMaterial* InMaterial,
                             const EStaticMeshType& InType)
    : IActor(InName, InParent), MeshResource(InMeshResource), Material(InMaterial), Type(InType)
    {
        ActorType = EActorType::STATIC_MESH;
    }

    void CStaticMesh::UIDrawActorDetails()
    {
        IActor::UIDrawActorDetails();

        ImGui::Text("Static mesh:");
        if (ImGui::BeginListBox("##listbox 2", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (int i = 0; i < GEngine.GetMeshesHolder().Length(); ++i)
            {
                resources::CMeshResource* CurrMeshResource = GEngine.GetMeshesHolder().GetByIndex(i);
                if(ImGui::Selectable(*CurrMeshResource->GetName(), MeshResource == CurrMeshResource))
                {
                    MeshResource = CurrMeshResource;
                }
                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (MeshResource == CurrMeshResource)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndListBox();
        }
    }

    float CStaticMesh::GetVerticalMidPoint() const
    {
        if (0.002f > fabs(MeshResource->MinPosY))
        {
            return (MeshResource->MaxPosY - MeshResource->MinPosY) * Transform.Scale.y / 2.f;
        }
        return 0;
    }

} // namespace lucid::scene
