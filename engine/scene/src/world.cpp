#include "scene/world.hpp"

#include "engine/engine.hpp"

#include "scene/render_scene.hpp"
#include "scene/renderer.hpp"

#include "scene/actors/static_mesh.hpp"
#include "scene/actors/skybox.hpp"
#include "scene/actors/lights.hpp"
#include "scene/actors/terrain.hpp"

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
            StaticMeshes.Add(InStaticMesh->ActorId, InStaticMesh);
        }
    }

    void CWorld::RemoveStaticMesh(const u32& InId) { StaticMeshes.Remove(InId); }

    void CWorld::AddDirectionalLight(CDirectionalLight* InLight)
    {
        if (AddActor(InLight))
        {
            DirectionalLights.Add(InLight->ActorId, InLight);
            AllLights.Add(InLight->ActorId, InLight);
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
            SpotLights.Add(InLight->ActorId, InLight);
            AllLights.Add(InLight->ActorId, InLight);
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
            PointLights.Add(InLight->ActorId, InLight);
            AllLights.Add(InLight->ActorId, InLight);
        }
    }
    void CWorld::RemovePointLight(const u32& InId)
    {
        PointLights.Remove(InId);
        AllLights.Remove(InId);
    }

    void CWorld::AddTerrain(CTerrain* InTerrain)
    {
        if (AddActor(InTerrain))
        {
            Terrains.Add(InTerrain->ActorId, InTerrain);
        }
    }

    void CWorld::RemoveTerrain(CTerrain* InTerrain) { Terrains.Remove(InTerrain->ActorId); }

    void CWorld::SetSkybox(CSkybox* InSkybox)
    {
        if (Skybox)
        {
            RemoveActorById(Skybox->ActorId, true);
        }

        if (InSkybox->ActorId == 0)
        {
            InSkybox->ActorId = NextActorId++;
        }

        Skybox = InSkybox;
        ActorById.Add(Skybox->ActorId, Skybox);
    }

    FRenderScene* CWorld::MakeRenderScene(CCamera* InCamera)
    {
        StaticRenderScene.AllLights         = AllLights;
        StaticRenderScene.DirectionalLights = DirectionalLights;
        StaticRenderScene.SpotLights        = SpotLights;
        StaticRenderScene.PointLights       = PointLights;
        StaticRenderScene.StaticMeshes      = StaticMeshes;
        StaticRenderScene.Skybox            = Skybox;
        StaticRenderScene.Terrains          = Terrains;
        return &StaticRenderScene;
    }

    IActor* CWorld::GetActorById(const u32& InActorId) { return ActorById.Get(InActorId); }

    u32 CWorld::AddActor(IActor* InActor)
    {
        if (InActor->ActorId == 0)
        {
            InActor->ActorId = NextActorId++;
        }
        else
        {
            NextActorId = NextActorId > InActor->ActorId ? NextActorId : InActor->ActorId + 1;
        }
        ActorById.Add(InActor->ActorId, InActor);
        LUCID_LOG(ELogLevel::INFO, "Actor '%s' added to the world", *InActor->Name);
        return InActor->ActorId;
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

            if (auto* StaticMesh = ActorAsset->LoadActor(World, &StaticMeshDescription))
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
            ActorAsset->LoadAssetResources();
            ActorAsset->LoadActor(World, &InWorldDescription.Skybox);
        }

        // Load lights
        for (const FDirectionalLightEntry& DirLightEntry : InWorldDescription.DirectionalLights)
        {
            CDirectionalLight* DirLight = GEngine.GetRenderer()->CreateDirectionalLight(
              DirLightEntry.Name, DirLightEntry.ParentId ? World->GetActorById(DirLightEntry.ParentId) : nullptr, World, DirLightEntry.bCastsShadow);

            DirLight->ActorId = DirLightEntry.Id;

            FTransform3D LightTransform;

            LightTransform.Translation = Float3ToVec(DirLightEntry.Postion);
            LightTransform.Rotation    = Float4ToQuat(DirLightEntry.Rotation);

            DirLight->Color        = Float3ToVec(DirLightEntry.Color);
            DirLight->Direction    = Float3ToVec(DirLightEntry.Direction);
            DirLight->LightUp      = Float3ToVec(DirLightEntry.LightUp);
            DirLight->bCastsShadow = DirLightEntry.bCastsShadow;
            DirLight->Illuminance  = DirLightEntry.Illuminance;
            DirLight->Left         = DirLightEntry.Left;
            DirLight->Right        = DirLightEntry.Right;
            DirLight->Bottom       = DirLightEntry.Bottom;
            DirLight->Top          = DirLightEntry.Top;
            DirLight->NearPlane    = DirLightEntry.NearPlane;
            DirLight->FarPlane     = DirLightEntry.FarPlane;
            DirLight->SetTransform(LightTransform);

            if (DirLightEntry.ParentId && !DirLight->Parent)
            {
                UnresolvedParents.Add(DirLight, DirLightEntry.ParentId);
            }

            World->AddDirectionalLight(DirLight);
        }

        for (const FSpotLightEntry& SpotLightEntry : InWorldDescription.SpotLights)
        {
            CSpotLight* SpotLight = GEngine.GetRenderer()->CreateSpotLight(
              SpotLightEntry.Name, SpotLightEntry.ParentId ? World->GetActorById(SpotLightEntry.ParentId) : nullptr, World, SpotLightEntry.bCastsShadow);

            SpotLight->ActorId = SpotLightEntry.Id;

            FTransform3D LightTransform;
            LightTransform.Translation = Float3ToVec(SpotLightEntry.Postion);
            LightTransform.Rotation    = Float4ToQuat(SpotLightEntry.Rotation);
            SpotLight->SetTransform(LightTransform);

            SpotLight->Color             = Float3ToVec(SpotLightEntry.Color);
            SpotLight->Direction         = Float3ToVec(SpotLightEntry.Direction);
            SpotLight->LightUp           = Float3ToVec(SpotLightEntry.LightUp);
            SpotLight->AttenuationRadius = SpotLightEntry.AttenuationRadius;
            SpotLight->InnerCutOffRad    = SpotLightEntry.InnerCutOffRad;
            SpotLight->OuterCutOffRad    = SpotLightEntry.OuterCutOffRad;
            SpotLight->bCastsShadow      = SpotLightEntry.bCastsShadow;
            SpotLight->LightUnit         = SpotLightEntry.LightUnit;
            SpotLight->LightSourceType   = SpotLightEntry.LightSourceType;
            SpotLight->RadiantPower      = SpotLightEntry.RadiantPowery;
            SpotLight->LuminousPower     = SpotLightEntry.LuminousPower;

            if (SpotLightEntry.ParentId && !SpotLight->Parent)
            {
                UnresolvedParents.Add(SpotLight, SpotLightEntry.ParentId);
            }

            World->AddSpotLight(SpotLight);
        }

        for (const FPointLightEntry& PointLightEntry : InWorldDescription.PointLights)
        {
            CPointLight* PointLight = GEngine.GetRenderer()->CreatePointLight(
              PointLightEntry.Name, PointLightEntry.ParentId ? World->GetActorById(PointLightEntry.ParentId) : nullptr, World, PointLightEntry.bCastsShadow);

            PointLight->ActorId = PointLightEntry.Id;

            FTransform3D LightTransform;
            LightTransform.Translation = Float3ToVec(PointLightEntry.Postion);
            LightTransform.Rotation    = Float4ToQuat(PointLightEntry.Rotation);
            PointLight->SetTransform(LightTransform);

            PointLight->Color = Float3ToVec(PointLightEntry.Color);

            PointLight->AttenuationRadius = PointLightEntry.AttenuationRadius;
            PointLight->bCastsShadow      = PointLightEntry.bCastsShadow;
            PointLight->LightUnit         = PointLightEntry.LightUnit;
            PointLight->LightSourceType   = PointLightEntry.LightSourceType;
            PointLight->RadiantPower      = PointLightEntry.RadiantPowery;
            PointLight->LuminousPower     = PointLightEntry.LuminousPower;

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

        // Terrains
        for (const FTerrainDescription& TerrainDescription : InWorldDescription.Terrains)
        {
            auto* ActorAsset = GEngine.GetActorsResources().Get(TerrainDescription.BaseActorResourceId);
            if (!ActorAsset)
            {
                LUCID_LOG(ELogLevel::WARN, "Failed to create terrain %s - no base asset", *TerrainDescription.Name);
                continue;
            }

            if (auto* Terrain = ActorAsset->LoadActor(World, &TerrainDescription))
            {
                if (TerrainDescription.ParentId && !Terrain->Parent)
                {
                    UnresolvedParents.Add(Terrain, TerrainDescription.ParentId);
                }
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
            DirLightEntry.Id                  = DirLight->ActorId;
            DirLightEntry.ParentId            = DirLight->Parent ? DirLight->Parent->ActorId : 0;
            DirLightEntry.Color               = VecToFloat3(DirLight->Color);
            DirLightEntry.Quality             = DirLight->Quality;
            DirLightEntry.Name                = DirLight->Name;
            DirLightEntry.Postion             = VecToFloat3(DirLight->GetTransform().Translation);
            DirLightEntry.Rotation            = { 0, 0, 0 };
            DirLightEntry.Direction           = VecToFloat3(DirLight->Direction);
            DirLightEntry.LightUp             = VecToFloat3(DirLight->LightUp);
            DirLightEntry.bCastsShadow        = DirLight->bCastsShadow;
            DirLightEntry.Illuminance         = DirLight->Illuminance;
            DirLightEntry.Left                = DirLight->Left;
            DirLightEntry.Right               = DirLight->Right;
            DirLightEntry.Bottom              = DirLight->Bottom;
            DirLightEntry.Top                 = DirLight->Top;
            DirLightEntry.NearPlane           = DirLight->NearPlane;
            DirLightEntry.FarPlane            = DirLight->FarPlane;

            OutWorldDescription.DirectionalLights.push_back(DirLightEntry);
        }

        // Save spot lights
        for (u32 i = 0; i < SpotLights.GetLength(); ++i)
        {
            const CSpotLight* SpotLight = SpotLights.GetByIndex(i);
            FSpotLightEntry   SpotLightEntry;
            SpotLightEntry.Id                = SpotLight->ActorId;
            SpotLightEntry.ParentId          = SpotLight->Parent ? SpotLight->Parent->ActorId : 0;
            SpotLightEntry.Color             = VecToFloat3(SpotLight->Color);
            SpotLightEntry.Quality           = SpotLight->Quality;
            SpotLightEntry.Name              = SpotLight->Name;
            SpotLightEntry.Postion           = VecToFloat3(SpotLight->GetTransform().Translation);
            SpotLightEntry.Rotation          = QuatToFloat4(SpotLight->GetTransform().Rotation);
            SpotLightEntry.Direction         = VecToFloat3(SpotLight->Direction);
            SpotLightEntry.AttenuationRadius = SpotLight->AttenuationRadius;
            SpotLightEntry.InnerCutOffRad    = SpotLight->InnerCutOffRad;
            SpotLightEntry.OuterCutOffRad    = SpotLight->OuterCutOffRad;
            SpotLightEntry.bCastsShadow      = SpotLight->ShadowMap != nullptr;
            SpotLightEntry.LightUnit         = SpotLight->LightUnit;
            SpotLightEntry.LightSourceType   = SpotLight->LightSourceType;
            SpotLightEntry.RadiantPowery     = SpotLight->RadiantPower;
            SpotLightEntry.LuminousPower     = SpotLight->LuminousPower;

            OutWorldDescription.SpotLights.push_back(SpotLightEntry);
        }

        // Save point lights
        for (u32 i = 0; i < PointLights.GetLength(); ++i)
        {
            const CPointLight* PointLight = PointLights.GetByIndex(i);
            FPointLightEntry   PointLightEntry;
            PointLightEntry.Id                = PointLight->ActorId;
            PointLightEntry.ParentId          = PointLight->Parent ? PointLight->Parent->ActorId : 0;
            PointLightEntry.Color             = VecToFloat3(PointLight->Color);
            PointLightEntry.Quality           = PointLight->Quality;
            PointLightEntry.Name              = PointLight->Name;
            PointLightEntry.Postion           = VecToFloat3(PointLight->GetTransform().Translation);
            PointLightEntry.AttenuationRadius = PointLight->AttenuationRadius;
            PointLightEntry.bCastsShadow      = PointLight->ShadowMap != nullptr;
            PointLightEntry.LightUnit         = PointLight->LightUnit;
            PointLightEntry.LightSourceType   = PointLight->LightSourceType;
            PointLightEntry.RadiantPowery     = PointLight->RadiantPower;
            PointLightEntry.LuminousPower     = PointLight->LuminousPower;

            OutWorldDescription.PointLights.push_back(PointLightEntry);
        }

        // Save terrains
        for (u32 i = 0; i < Terrains.GetLength(); ++i)
        {
            const CTerrain*     Terrain = Terrains.GetByIndex(i);
            FTerrainDescription TerrainEntry;
            TerrainEntry.Name                = Terrain->Name;
            TerrainEntry.ParentId            = Terrain->Parent ? Terrain->Parent->ActorId : 0;
            TerrainEntry.ParentId            = Terrain->Parent ? Terrain->Parent->ActorId : 0;
            TerrainEntry.BaseActorResourceId = Terrain->BaseActorAsset->AssetId;
            TerrainEntry.Postion             = VecToFloat3(Terrain->GetTransform().Translation);
            TerrainEntry.Rotation            = QuatToFloat4(Terrain->GetTransform().Rotation);
            TerrainEntry.Scale               = VecToFloat3(Terrain->GetTransform().Scale);
            OutWorldDescription.Terrains.push_back(TerrainEntry);
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

            if (Skybox && InActorId == Skybox->ActorId)
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
