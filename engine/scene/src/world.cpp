#include "scene/world.hpp"

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

    /** Id for the skybox renderable, as there is always only one skybox */
    static const u32 SkyboxRenderableId = 4294967295;
    
    void CWorld::Init() {}

    void CWorld::AddStaticMesh(CStaticMesh* InStaticMesh)
    {
        if (AddActor(InStaticMesh))
        {
            arrput(StaticMeshes, InStaticMesh);            
        }
    }

    void CWorld::AddDirectionalLight(CDirectionalLight* InLight)
    {
        if (AddActor(InLight))
        {
            arrput(DirectionalLights, InLight);
            arrput(AllLights, InLight);
        }
    }

    void CWorld::AddSpotLight(CSpotLight* InLight)
    {
        if (AddActor(InLight))
        {
            arrput(SpotLights, InLight);
            arrput(AllLights, InLight);
        }
    }

    void CWorld::AddPointLight(CPointLight* InLight)
    {
        if (AddActor(InLight))
        {
            arrput(PointLights, InLight);
            arrput(AllLights, InLight);
        }
    }
    
    void CWorld::SetSkybox(CSkybox* InSkybox)
    {
        if (Skybox)
        {
            Skybox->Id = 0;
        }
        
        Skybox = InSkybox;
        Skybox->Id = SkyboxRenderableId;
    }

    FRenderScene* CWorld::MakeRenderScene(CCamera* InCamera)
    {
        StaticRenderScene.AllLights = AllLights;
        StaticRenderScene.DirectionalLights = DirectionalLights;
        StaticRenderScene.SpotLights = SpotLights;
        StaticRenderScene.PointLights = PointLights;
        StaticRenderScene.StaticMeshes = StaticMeshes;
        StaticRenderScene.Skybox = Skybox;
        return &StaticRenderScene;
    }

    IActor* CWorld::GetActorById(const u32& InActorId)
    {
        if (hmgeti(ActorById, InActorId) != -1)
        {
            return hmget(ActorById, InActorId);
        }
        return nullptr;
    }

    u32 CWorld::AddActor(IActor* InActor)
    {
        // If actor doesn't have an id yet or it's not in the map
        if (InActor->Id == 0 || hmgeti(ActorById, InActor->Id) == -1)
        {
            InActor->Id = NextActorId++;
            hmput(ActorById, InActor->Id, InActor);
            LUCID_LOG(ELogLevel::INFO, "Actor '%s' added to the world", *InActor->Name);
            return InActor->Id;
        }
        LUCID_LOG(ELogLevel::WARN, "Duplicate actor '%s', id %d", *InActor->Name, InActor->Id)
        return 0;
    }

    CWorld* LoadWorldFromJSONFile(const FString& InFilePath)
    {
        FWorldDescription WorldDescription;
        ReadFromJSONFile(WorldDescription, *InFilePath);
        return LoadWorld(WorldDescription);
    }
    
    CWorld* LoadWorldFromBinaryFile(const FString& InFilePath)
    {
        FWorldDescription WorldDescription;
        ReadFromBinaryFile(WorldDescription, *InFilePath);
        return LoadWorld(WorldDescription);         
    }

    CWorld* LoadWorld(const FWorldDescription& InWorldDescription)
    {
        auto World = new CWorld;

        // Load static meshes
        FStaticMeshDescription StaticMeshDescription;
        for (const FActorEntry& StaticMeshEntry : InWorldDescription.StaticMeshes)
        {
            if (ReadFromJSONFile(StaticMeshDescription, *StaticMeshEntry.ResourcePath))
            {
                CMaterial* Material = GEngine.GetMaterialsHolder().Get(*StaticMeshDescription.MaterialName);
                resources::CMeshResource* MeshResource = GEngine.GetMeshesHolder().Get(*StaticMeshDescription.MeshResourceName);

                auto StaticMesh = new CStaticMesh {
                    StaticMeshEntry.Name,
                    World->GetActorById(StaticMeshEntry.ParentId), 
                    MeshResource,
                    Material,
                    StaticMeshDescription.Type
                };

                StaticMesh->Id = StaticMeshEntry.Id;
                StaticMesh->Transform.Translation = Float3ToVec(StaticMeshEntry.Postion);
                StaticMesh->Transform.Rotation = Float4ToQuat(StaticMeshEntry.Rotation);
                StaticMesh->Transform.Scale = Float3ToVec(StaticMeshEntry.Scale);
                StaticMesh->bVisible = StaticMeshEntry.bVisible;
                StaticMesh->ResourcePath = StaticMeshEntry.ResourcePath;
                
                World->AddStaticMesh(StaticMesh);
            }
        }

        // Load skybox
        {
            const resources::CTextureResource* FaceTextures[6];
            for (u8 i = 0; i < 6; ++i)
            {
                FaceTextures[i] = GEngine.GetTexturesHolder().Get(*InWorldDescription.Skybox.FacesTextures[i]);
            }
            resources::CTextureResource* FirstTex = GEngine.GetTexturesHolder().Get(*InWorldDescription.Skybox.FacesTextures[0]);
            CSkybox* Skybox = CreateSkybox(FaceTextures, FirstTex->Width, FirstTex->Height, "Skybox");
            World->SetSkybox(Skybox);
        }

        // Load lights
        for (const FDirectionalLightEntry& DirLightEntry : InWorldDescription.DirectionalLights)
        {
            CDirectionalLight* DirLight = GEngine.GetRenderer()->CreateDirectionalLight(DirLightEntry.Name, World->GetActorById(DirLightEntry.ParentId), DirLightEntry.bCastsShadow);

            DirLight->Id = DirLightEntry.Id;

            DirLight->Transform.Translation = Float3ToVec(DirLightEntry.Postion);
            DirLight->Transform.Rotation = Float4ToQuat(DirLightEntry.Rotation);

            DirLight->Color = Float3ToVec(DirLightEntry.Color);
            DirLight->Direction = Float3ToVec(DirLightEntry.Direction);
            DirLight->LightUp = Float3ToVec(DirLightEntry.LightUp);

            World->AddDirectionalLight(DirLight);
        }

        for (const FSpotLightEntry& SpotLightEntry : InWorldDescription.SpotLights)
        {
            CSpotLight* SpotLight = GEngine.GetRenderer()->CreateSpotLight(SpotLightEntry.Name, World->GetActorById(SpotLightEntry.ParentId), SpotLightEntry.bCastsShadow);

            SpotLight->Id = SpotLightEntry.Id;

            SpotLight->Transform.Translation = Float3ToVec(SpotLightEntry.Postion);
            SpotLight->Transform.Rotation = Float4ToQuat(SpotLightEntry.Rotation);

            SpotLight->Color = Float3ToVec(SpotLightEntry.Color);
            SpotLight->Direction = Float3ToVec(SpotLightEntry.Direction);
            SpotLight->LightUp = Float3ToVec(SpotLightEntry.LightUp);

            SpotLight->Constant = SpotLightEntry.Constant; 
            SpotLight->Linear = SpotLightEntry.Linear; 
            SpotLight->Quadratic = SpotLightEntry.Quadratic; 
            SpotLight->InnerCutOffRad = SpotLightEntry.InnerCutOffRad; 
            SpotLight->OuterCutOffRad = SpotLightEntry.OuterCutOffRad; 
            
            World->AddSpotLight(SpotLight);
        }

        for (const FPointLightEntry& PointLightEntry : InWorldDescription.PointLights)
        {
            CPointLight* PointLight = GEngine.GetRenderer()->CreatePointLight(PointLightEntry.Name, World->GetActorById(PointLightEntry.ParentId), PointLightEntry.bCastsShadow);

            PointLight->Id = PointLightEntry.Id;

            PointLight->Transform.Translation = Float3ToVec(PointLightEntry.Postion);
            PointLight->Transform.Rotation = Float4ToQuat(PointLightEntry.Rotation);

            PointLight->Color = Float3ToVec(PointLightEntry.Color);

            PointLight->Constant = PointLightEntry.Constant; 
            PointLight->Linear = PointLightEntry.Linear; 
            PointLight->Quadratic = PointLightEntry.Quadratic; 
            PointLight->NearPlane = PointLightEntry.NearPlane; 
            PointLight->FarPlane = PointLightEntry.FarPlane; 
            
            World->AddPointLight(PointLight);
        }

        return World;
    }

    void CWorld::CreateWorldDescription(FWorldDescription& InWorldDescription) const
    {
        // Save static meshes
        FActorEntry StaticMeshEntry;
        for (u32 i = 0; i < arrlen(StaticMeshes); ++i)
        {
            const CStaticMesh* StaticMesh = StaticMeshes[i];
            StaticMeshEntry.Id = StaticMesh->Id; 
            StaticMeshEntry.ParentId = StaticMesh->Parent ? StaticMesh->Parent->Id : 0;
            StaticMeshEntry.Name = StaticMesh->Name;
            StaticMeshEntry.ResourcePath = StaticMesh->ResourcePath;
            StaticMeshEntry.Postion = VecToFloat3(StaticMesh->Transform.Translation);
            StaticMeshEntry.Rotation = QuatToFloat4(StaticMesh->Transform.Rotation);
            StaticMeshEntry.Scale = VecToFloat3(StaticMesh->Transform.Scale);
            StaticMeshEntry.bVisible = StaticMesh->bVisible;

            InWorldDescription.StaticMeshes.push_back(StaticMeshEntry);
        }

        // Save skybox
        if (Skybox)
        {
            for (u8 i = 0; i < 6; ++i)
            {
                InWorldDescription.Skybox.FacesTextures[i] = CopyToString(*Skybox->FaceTextures[i]->GetName(), Skybox->FaceTextures[i]->GetName().GetLength());
            }
        }

        // Save dir lights
        FDirectionalLightEntry DirLightEntry;
        for (u32 i = 0; i < arrlen(DirectionalLights); ++i)
        {
            const CDirectionalLight* DirLight = DirectionalLights[i];
            DirLightEntry.Id = DirLight->Id;
            DirLightEntry.ParentId = DirLight->Parent ? DirLight->Parent->Id : 0;
            DirLightEntry.Color = VecToFloat3(DirLight->Color);
            DirLightEntry.Quality = DirLight->Quality;
            DirLightEntry.Name = DirLight->Name;
            DirLightEntry.Postion = { 0, 0, 0};
            DirLightEntry.Rotation = {0, 0, 0};
            DirLightEntry.Direction = VecToFloat3(DirLight->Direction);
            DirLightEntry.LightUp = VecToFloat3(DirLight->LightUp);

            InWorldDescription.DirectionalLights.push_back(DirLightEntry);
        }

        // Save spot lights
        FSpotLightEntry SpotLightEntry;
        for (u32 i = 0; i < arrlen(SpotLights); ++i)
        {
            const CSpotLight* SpotLight = SpotLights[i];
            SpotLightEntry.Id = SpotLight->Id;
            SpotLightEntry.ParentId = SpotLight->Parent ? SpotLight->Parent->Id : 0;
            SpotLightEntry.Color = VecToFloat3(SpotLight->Color);
            SpotLightEntry.Quality = SpotLight->Quality;
            SpotLightEntry.Name = SpotLight->Name;
            SpotLightEntry.Postion = { 0, 0, 0};
            SpotLightEntry.Rotation = {0, 0, 0};
            SpotLightEntry.Direction = VecToFloat3(SpotLight->Direction);
            SpotLightEntry.Constant = SpotLight->Constant;
            SpotLightEntry.Linear = SpotLight->Linear;
            SpotLightEntry.Quadratic = SpotLight->Quadratic;
            SpotLightEntry.InnerCutOffRad = SpotLight->InnerCutOffRad;
            SpotLightEntry.OuterCutOffRad = SpotLight->OuterCutOffRad;

            InWorldDescription.SpotLights.push_back(SpotLightEntry);
        }

        // Save point lights
        FPointLightEntry PointLightEntry;
        for (u32 i = 0; i < arrlen(PointLights); ++i)
        {
            const CPointLight* PointLight = PointLights[i];
            PointLightEntry.Id = PointLight->Id;
            PointLightEntry.ParentId = PointLight->Parent ? PointLight->Parent->Id : 0;
            PointLightEntry.Color = VecToFloat3(PointLight->Color);
            PointLightEntry.Quality = PointLight->Quality;
            PointLightEntry.Name = PointLight->Name;
            PointLightEntry.Postion = { 0, 0, 0};
            PointLightEntry.Rotation = {0, 0, 0};
            PointLightEntry.Constant = PointLight->Constant;
            PointLightEntry.Linear = PointLight->Linear;
            PointLightEntry.Quadratic = PointLight->Quadratic;
            PointLightEntry.NearPlane = PointLight->NearPlane;
            PointLightEntry.FarPlane = PointLight->FarPlane;

            InWorldDescription.PointLights.push_back(PointLightEntry);
        }
    }

    void CWorld::SaveToJSONFile(const FDString& InFilePath) const
    {
        FWorldDescription WorldDescription {};
        CreateWorldDescription(WorldDescription);
        WriteToJSONFile(WorldDescription, *InFilePath);

        if (Skybox)
        {
            for (u8 i = 0; i < 6; ++i)
            {
                WorldDescription.Skybox.FacesTextures[i].Free();
            }            
        }
    }

    void CWorld::SaveToBinaryFile(const FDString& InFilePath) const
    {
        FWorldDescription WorldDescription;
        CreateWorldDescription(WorldDescription);
        // WriteToBinaryFile(WorldDescription, *InFilePath);

        if (Skybox)
        {
            for (u8 i = 0; i < 6; ++i)
            {
                WorldDescription.Skybox.FacesTextures[i].Free();
            }            
        }
    }
} // namespace lucid::scene
