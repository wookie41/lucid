#include "scene/world.hpp"

#include <scene/material.hpp>

#include "resources/texture_resource.hpp"
#include "engine/engine.hpp"

#include "scene/render_scene.hpp"
#include "scene/renderer.hpp"

#include "scene/actors/static_mesh.hpp"
#include "scene/actors/skybox.hpp"
#include "scene/actors/lights.hpp"

#include "stb_ds.h"
#include "common/log.hpp"

#include "common/types.hpp"

#include "schemas/types.hpp"
#include "schemas/json.hpp"
#include "schemas/binary.hpp"

namespace lucid::scene
{
    FRenderScene StaticRenderScene;

    void CWorld::Init() {}

    void CWorld::AddStaticMesh(CStaticMesh* InStaticMesh)
    {
        if (AddActor(InStaticMesh))
        {
            StaticMeshes.Add(InStaticMesh->Id, InStaticMesh);
        }
    }

    void CWorld::RemoveStaticMesh(const u32& InId) { StaticMeshes.Remove(InId); }

    void CWorld::AddDirectionalLight(CDirectionalLight* InLight)
    {
        if (AddActor(InLight))
        {
            DirectionalLights.Add(InLight->Id, InLight);
            AllLights.Add(InLight->Id, InLight);
        }
    }

    void CWorld::RemoveDirectionalLight(const u32& InId)
    {
        DirectionalLights.Remove(InId);
        AllLights.Remove(InId);
    }

    void CWorld::AddSpotLight(CSpotLight* InLight)
    {
        if (AddActor(InLight))
        {
            SpotLights.Add(InLight->Id, InLight);
            AllLights.Add(InLight->Id, InLight);
        }
    }
    void CWorld::RemoveSpotLight(const u32& InId)
    {
        SpotLights.Remove(InId);
        AllLights.Remove(InId);
    }

    void CWorld::AddPointLight(CPointLight* InLight)
    {
        if (AddActor(InLight))
        {
            PointLights.Add(InLight->Id, InLight);
            AllLights.Add(InLight->Id, InLight);
        }
    }
    void CWorld::RemovePointLight(const u32& InId)
    {
        PointLights.Remove(InId);
        AllLights.Remove(InId);
    }

    void CWorld::SetSkybox(CSkybox* InSkybox)
    {
        if (Skybox)
        {
            RemoveActorById(Skybox->Id, true);
        }

        if (InSkybox->Id == 0)
        {
            InSkybox->Id = NextActorId++;
        }

        Skybox = InSkybox;
        ActorById.Add(Skybox->Id, Skybox);
    }

    FRenderScene* CWorld::MakeRenderScene(CCamera* InCamera)
    {
        StaticRenderScene.AllLights         = AllLights;
        StaticRenderScene.DirectionalLights = DirectionalLights;
        StaticRenderScene.SpotLights        = SpotLights;
        StaticRenderScene.PointLights       = PointLights;
        StaticRenderScene.StaticMeshes      = StaticMeshes;
        StaticRenderScene.Skybox            = Skybox;
        return &StaticRenderScene;
    }

    IActor* CWorld::GetActorById(const u32& InActorId) { return ActorById.Get(InActorId); }

    u32 CWorld::AddActor(IActor* InActor)
    {
        if (InActor->Id == 0)
        {
            InActor->Id = NextActorId++;
        }
        else
        {
            NextActorId = NextActorId > InActor->Id ? NextActorId : InActor->Id + 1;
        }
        ActorById.Add(InActor->Id, InActor);
        LUCID_LOG(ELogLevel::INFO, "Actor '%s' added to the world", *InActor->Name);
        return InActor->Id;
    }

    CWorld* LoadWorldFromJSONFile(const FString& InFilePath)
    {
        FWorldDescription WorldDescription;
        if (ReadFromJSONFile(WorldDescription, *InFilePath))
        {
            return LoadWorld(WorldDescription);
        }
        LUCID_LOG(ELogLevel::WARN, "Failed to load world from JSON file %s", *InFilePath);
        return nullptr;
    }

    CWorld* LoadWorldFromBinaryFile(const FString& InFilePath)
    {
        FWorldDescription WorldDescription;
        if (ReadFromBinaryFile(WorldDescription, *InFilePath))
        {
            return LoadWorld(WorldDescription);
        }
        LUCID_LOG(ELogLevel::WARN, "Failed to load world from binary file %s", *InFilePath);
        return nullptr;
    }

    CWorld* LoadWorld(const FWorldDescription& InWorldDescription)
    {
        auto World = new CWorld;
        World->Init();

        // Helper map used to resolve parents once we load the world, needed because parents might be loaded
        // after children which result in null parents
        FHashMap<IActor*, u32> UnresolvedParents;

        // Load static meshes
        for (const FStaticMeshDescription& StaticMeshDescription : InWorldDescription.StaticMeshes)
        {
            auto* ActorAsset = GEngine.GetActorsResources().Get(StaticMeshDescription.BaseActorResourceId);
            if (!ActorAsset)
            {
                LUCID_LOG(ELogLevel::WARN, "Failed to create static mesh %s - no base asset", *StaticMeshDescription.Name);
                continue;
            }

            ActorAsset->LoadAsset();
            if (auto* StaticMesh = CStaticMesh::CreateActor((CStaticMesh*)ActorAsset, World, StaticMeshDescription))
            {
                if (StaticMeshDescription.ParentId && !StaticMesh->Parent)
                {
                    UnresolvedParents.Add(StaticMesh, StaticMeshDescription.ParentId);
                }
            }
        }

        if (InWorldDescription.Skybox.BaseActorResourceId != sole::INVALID_UUID)
        {
            auto* ActorAsset = GEngine.GetActorsResources().Get(InWorldDescription.Skybox.BaseActorResourceId);
            ActorAsset->LoadAsset();
            CSkybox::CreateActor((CSkybox*)ActorAsset, World, InWorldDescription.Skybox);
        }

        // Load lights
        for (const FDirectionalLightEntry& DirLightEntry : InWorldDescription.DirectionalLights)
        {
            CDirectionalLight* DirLight =
              GEngine.GetRenderer()->CreateDirectionalLight(DirLightEntry.Name,
                                                            DirLightEntry.ParentId ? World->GetActorById(DirLightEntry.ParentId) : nullptr,
                                                            World,
                                                            DirLightEntry.bCastsShadow);

            DirLight->Id = DirLightEntry.Id;

            DirLight->Transform.Translation = Float3ToVec(DirLightEntry.Postion);
            DirLight->Transform.Rotation    = Float4ToQuat(DirLightEntry.Rotation);

            DirLight->Color             = Float3ToVec(DirLightEntry.Color);
            DirLight->Direction         = Float3ToVec(DirLightEntry.Direction);
            DirLight->LightUp           = Float3ToVec(DirLightEntry.LightUp);
            DirLight->bShouldCastShadow = DirLightEntry.bCastsShadow;

            if (DirLightEntry.ParentId && !DirLight->Parent)
            {
                UnresolvedParents.Add(DirLight, DirLightEntry.ParentId);
            }

            World->AddDirectionalLight(DirLight);
        }

        for (const FSpotLightEntry& SpotLightEntry : InWorldDescription.SpotLights)
        {
            CSpotLight* SpotLight =
              GEngine.GetRenderer()->CreateSpotLight(SpotLightEntry.Name,
                                                     SpotLightEntry.ParentId ? World->GetActorById(SpotLightEntry.ParentId) : nullptr,
                                                     World,
                                                     SpotLightEntry.bCastsShadow);

            SpotLight->Id = SpotLightEntry.Id;

            SpotLight->Transform.Translation = Float3ToVec(SpotLightEntry.Postion);
            SpotLight->Transform.Rotation    = Float4ToQuat(SpotLightEntry.Rotation);

            SpotLight->Color     = Float3ToVec(SpotLightEntry.Color);
            SpotLight->Direction = Float3ToVec(SpotLightEntry.Direction);
            SpotLight->LightUp   = Float3ToVec(SpotLightEntry.LightUp);

            SpotLight->Constant          = SpotLightEntry.Constant;
            SpotLight->Linear            = SpotLightEntry.Linear;
            SpotLight->Quadratic         = SpotLightEntry.Quadratic;
            SpotLight->InnerCutOffRad    = SpotLightEntry.InnerCutOffRad;
            SpotLight->OuterCutOffRad    = SpotLightEntry.OuterCutOffRad;
            SpotLight->bShouldCastShadow = SpotLightEntry.bCastsShadow;

            if (SpotLightEntry.ParentId && !SpotLight->Parent)
            {
                UnresolvedParents.Add(SpotLight, SpotLightEntry.ParentId);
            }

            World->AddSpotLight(SpotLight);
        }

        for (const FPointLightEntry& PointLightEntry : InWorldDescription.PointLights)
        {
            CPointLight* PointLight =
              GEngine.GetRenderer()->CreatePointLight(PointLightEntry.Name,
                                                      PointLightEntry.ParentId ? World->GetActorById(PointLightEntry.ParentId) : nullptr,
                                                      World,
                                                      PointLightEntry.bCastsShadow);

            PointLight->Id = PointLightEntry.Id;

            PointLight->Transform.Translation = Float3ToVec(PointLightEntry.Postion);
            PointLight->Transform.Rotation    = Float4ToQuat(PointLightEntry.Rotation);

            PointLight->Color = Float3ToVec(PointLightEntry.Color);

            PointLight->Constant          = PointLightEntry.Constant;
            PointLight->Linear            = PointLightEntry.Linear;
            PointLight->Quadratic         = PointLightEntry.Quadratic;
            PointLight->bShouldCastShadow = PointLightEntry.bCastsShadow;

            if (PointLightEntry.ParentId && !PointLight->Parent)
            {
                UnresolvedParents.Add(PointLight, PointLightEntry.ParentId);
            }

            World->AddPointLight(PointLight);
        }

        // Setup parents
        for (int i = 0; i < UnresolvedParents.GetLength(); ++i)
        {
            auto ActorToParentID = UnresolvedParents.GetEntryByIndex(i);
            if (auto* Parent = World->GetActorById(ActorToParentID.value))
            {
                Parent->AddChild(ActorToParentID.key);
            }
            else
            {
                LUCID_LOG(ELogLevel::WARN, "No parent %d found for actor %s", ActorToParentID.value, *ActorToParentID.key->Name)
            }
        }

        UnresolvedParents.FreeAll();
        return World;
    }

    void CWorld::CreateWorldDescription(FWorldDescription& OutWorldDescription) const
    {
        // Save static meshes
        for (u32 i = 0; i < StaticMeshes.GetLength(); ++i)
        {
            FStaticMeshDescription StaticMeshEntry;
            const CStaticMesh*     StaticMesh = StaticMeshes.GetByIndex(i);
            StaticMesh->FillDescription(StaticMeshEntry);
            OutWorldDescription.StaticMeshes.push_back(StaticMeshEntry);
        }

        // Save skybox
        if (Skybox)
        {
            Skybox->FillDescription(OutWorldDescription.Skybox);
        }

        // Save dir lights
        for (u32 i = 0; i < DirectionalLights.GetLength(); ++i)
        {
            FDirectionalLightEntry   DirLightEntry;
            const CDirectionalLight* DirLight = DirectionalLights.GetByIndex(i);
            DirLightEntry.Id                  = DirLight->Id;
            DirLightEntry.ParentId            = DirLight->Parent ? DirLight->Parent->Id : 0;
            DirLightEntry.Color               = VecToFloat3(DirLight->Color);
            DirLightEntry.Quality             = DirLight->Quality;
            DirLightEntry.Name                = DirLight->Name;
            DirLightEntry.Postion             = { 0, 0, 0 };
            DirLightEntry.Rotation            = { 0, 0, 0 };
            DirLightEntry.Direction           = VecToFloat3(DirLight->Direction);
            DirLightEntry.LightUp             = VecToFloat3(DirLight->LightUp);
            DirLightEntry.bCastsShadow        = DirLight->ShadowMap != nullptr;

            OutWorldDescription.DirectionalLights.push_back(DirLightEntry);
        }

        // Save spot lights
        for (u32 i = 0; i < SpotLights.GetLength(); ++i)
        {
            const CSpotLight* SpotLight = SpotLights.GetByIndex(i);
            FSpotLightEntry   SpotLightEntry;
            SpotLightEntry.Id             = SpotLight->Id;
            SpotLightEntry.ParentId       = SpotLight->Parent ? SpotLight->Parent->Id : 0;
            SpotLightEntry.Color          = VecToFloat3(SpotLight->Color);
            SpotLightEntry.Quality        = SpotLight->Quality;
            SpotLightEntry.Name           = SpotLight->Name;
            SpotLightEntry.Postion        = VecToFloat3(SpotLight->Transform.Translation);
            SpotLightEntry.Rotation       = QuatToFloat4(SpotLight->Transform.Rotation);
            SpotLightEntry.Direction      = VecToFloat3(SpotLight->Direction);
            SpotLightEntry.Constant       = SpotLight->Constant;
            SpotLightEntry.Linear         = SpotLight->Linear;
            SpotLightEntry.Quadratic      = SpotLight->Quadratic;
            SpotLightEntry.InnerCutOffRad = SpotLight->InnerCutOffRad;
            SpotLightEntry.OuterCutOffRad = SpotLight->OuterCutOffRad;
            SpotLightEntry.bCastsShadow   = SpotLight->ShadowMap != nullptr;

            OutWorldDescription.SpotLights.push_back(SpotLightEntry);
        }

        // Save point lights
        for (u32 i = 0; i < PointLights.GetLength(); ++i)
        {
            const CPointLight* PointLight = PointLights.GetByIndex(i);
            FPointLightEntry   PointLightEntry;
            PointLightEntry.Id           = PointLight->Id;
            PointLightEntry.ParentId     = PointLight->Parent ? PointLight->Parent->Id : 0;
            PointLightEntry.Color        = VecToFloat3(PointLight->Color);
            PointLightEntry.Quality      = PointLight->Quality;
            PointLightEntry.Name         = PointLight->Name;
            PointLightEntry.Postion      = VecToFloat3(PointLight->Transform.Translation);
            PointLightEntry.Constant     = PointLight->Constant;
            PointLightEntry.Linear       = PointLight->Linear;
            PointLightEntry.Quadratic    = PointLight->Quadratic;
            PointLightEntry.bCastsShadow = PointLight->ShadowMap != nullptr;

            OutWorldDescription.PointLights.push_back(PointLightEntry);
        }
    }

    void CWorld::SaveToJSONFile(const FString& InFilePath) const
    {
        FWorldDescription WorldDescription{};
        CreateWorldDescription(WorldDescription);
        WriteToJSONFile(WorldDescription, *InFilePath);
    }

    void CWorld::SaveToBinaryFile(const FString& InFilePath) const
    {
        FWorldDescription WorldDescription;
        CreateWorldDescription(WorldDescription);
        WriteToBinaryFile(WorldDescription, *InFilePath);
    }

    IActor* CWorld::RemoveActorById(const u32& InActorId, const bool& InbHardRemove)
    {
        if (ActorById.Contains(InActorId))
        {
            IActor* Actor = ActorById.Get(InActorId);
            Actor->OnRemoveFromWorld(InbHardRemove);
            ActorById.Remove(InActorId);

            if (Skybox && InActorId == Skybox->Id)
            {
                Skybox = nullptr;
            }
            
            return Actor;
        }

        return nullptr;
    }

    void CWorld::Unload()
    {
        for (u32 i = 0; i < ActorById.GetLength(); ++i)
        {
            IActor* Actor = ActorById.GetByIndex(i);
            Actor->OnRemoveFromWorld(true);
            delete Actor;
        }

        ActorById.FreeAll();
        StaticMeshes.FreeAll();
        DirectionalLights.FreeAll();
        SpotLights.FreeAll();
        PointLights.FreeAll();
        AllLights.FreeAll();
        if (Skybox)
        {
            Skybox = nullptr;
        }
    }

} // namespace lucid::scene
