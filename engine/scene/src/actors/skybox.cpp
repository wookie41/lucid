#include "scene/actors/skybox.hpp"

#include <devices/gpu/texture_enums.hpp>

#include "engine/engine.hpp"
#include "devices/gpu/cubemap.hpp"
#include "resources/texture_resource.hpp"
#include "schemas/json.hpp"
#include "scene/world.hpp"

namespace lucid::scene
{
    CSkybox*
    CreateSkybox(resources::CTextureResource* InFaceTextures[6], CWorld* InWorld, const u32& InWidth, const u32& InHeight, const FString& InName)
    {
        const void* FacesData[6];
        for (u8 i = 0; i < 6; ++i)
        {
            FacesData[i] = InFaceTextures[i]->TextureData;
        }

        gpu::CCubemap* SkyboxCubemap = gpu::CreateCubemap(InWidth,
                                                          InHeight,
                                                          gpu::ETextureDataFormat::SRGB,
                                                          gpu::ETexturePixelFormat::RGB,
                                                          gpu::ETextureDataType::UNSIGNED_BYTE,
                                                          FacesData,
                                                          InName,
                                                          gpu::EMinTextureFilter::NEAREST,
                                                          gpu::EMagTextureFilter::NEAREST,
                                                          gpu::EWrapTextureFilter::CLAMP_TO_EDGE,
                                                          gpu::EWrapTextureFilter::CLAMP_TO_EDGE,
                                                          gpu::EWrapTextureFilter::CLAMP_TO_EDGE,
                                                          { 0, 0, 0, 0 });

        return new CSkybox{ CopyToString(*InName, InName.GetLength()), nullptr, InWorld, SkyboxCubemap, InWidth, InHeight, InFaceTextures };
    }

    CSkybox::CSkybox(const FDString&              InName,
                     IActor*                      InParent,
                     CWorld*                      InWorld,
                     gpu::CCubemap*               InSkyboxCubemap,
                     const u32&                   InWidth,
                     const u32&                   InHeight,
                     resources::CTextureResource* InFaceTextures[6])
    : IActor(InName, InParent, InWorld), SkyboxCubemap(InSkyboxCubemap), Width(InWidth), Height(InHeight)
    {
        if (InFaceTextures)
        {
            for (u8 i = 0; i < 6; ++i)
            {
                FaceTextures[i] = InFaceTextures[i];
            }
        }
    }

    void CSkybox::UIDrawActorDetails() {}

    float CSkybox::GetVerticalMidPoint() const { return 0; }

    CSkybox* CSkybox::CreateActor(CSkybox* BaseActorResource, CWorld* InWorld, const FSkyboxDescription& InSkyboxDescription)
    {
        resources::CTextureResource* SkyboxFaces[6];

        if (InSkyboxDescription.RightFaceTextureID.bChanged)
        {
            SkyboxFaces[0] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.RightFaceTextureID.Value);
        }
        else
        {
            SkyboxFaces[0] = BaseActorResource->FaceTextures[0];
        }

        if (InSkyboxDescription.LeftFaceTextureID.bChanged)
        {
            SkyboxFaces[1] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.LeftFaceTextureID.Value);
        }
        else
        {
            SkyboxFaces[1] = BaseActorResource->FaceTextures[1];
        }

        if (InSkyboxDescription.TopFaceTextureID.bChanged)
        {
            SkyboxFaces[2] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.TopFaceTextureID.Value);
        }
        else
        {
            SkyboxFaces[2] = BaseActorResource->FaceTextures[2];
        }

        if (InSkyboxDescription.BottomFaceTextureID.bChanged)
        {
            SkyboxFaces[3] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.BottomFaceTextureID.Value);
        }
        else
        {
            SkyboxFaces[3] = BaseActorResource->FaceTextures[3];
        }

        if (InSkyboxDescription.FrontFaceTextureID.bChanged)
        {
            SkyboxFaces[4] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.FrontFaceTextureID.Value);
        }
        else
        {
            SkyboxFaces[4] = BaseActorResource->FaceTextures[4];
        }

        if (InSkyboxDescription.BackFaceTextureID.bChanged)
        {
            SkyboxFaces[5] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.BackFaceTextureID.Value);
        }
        else
        {
            SkyboxFaces[5] = BaseActorResource->FaceTextures[5];
        }

        auto* Skybox = CreateSkybox(SkyboxFaces, InWorld, SkyboxFaces[0]->Width, SkyboxFaces[0]->Height, "Skybox");
        InWorld->SetSkybox(Skybox);
        Skybox->BaseSkyboxResource = BaseActorResource;
        return Skybox;
    }

    void CSkybox::InternalSaveToResourceFile(const FString& InFilePath)
    {
        FSkyboxDescription SkyboxDescription;
        FillDescription(SkyboxDescription);
        WriteToJSONFile(SkyboxDescription, *InFilePath);
        GEngine.GetActorsDatabase().Entries.push_back({ ResourceId, ResourcePath, EActorType::SKYBOX });
    }

    void CSkybox::FillDescription(FSkyboxDescription& OutDescription) const
    {
        if (BaseSkyboxResource)
        {
            OutDescription.BaseActorResourceId = BaseSkyboxResource->ResourceId;

            if (BaseSkyboxResource->FaceTextures[0] != FaceTextures[0])
            {
                OutDescription.RightFaceTextureID.bChanged = true;
                OutDescription.RightFaceTextureID.Value    = FaceTextures[0]->GetID();
            }

            if (BaseSkyboxResource->FaceTextures[1] != FaceTextures[1])
            {
                OutDescription.LeftFaceTextureID.bChanged = true;
                OutDescription.LeftFaceTextureID.Value    = FaceTextures[1]->GetID();
            }

            if (BaseSkyboxResource->FaceTextures[2] != FaceTextures[2])
            {
                OutDescription.TopFaceTextureID.bChanged = true;
                OutDescription.TopFaceTextureID.Value    = FaceTextures[2]->GetID();
            }

            if (BaseSkyboxResource->FaceTextures[3] != FaceTextures[3])
            {
                OutDescription.BottomFaceTextureID.bChanged = true;
                OutDescription.BottomFaceTextureID.Value    = FaceTextures[3]->GetID();
            }

            if (BaseSkyboxResource->FaceTextures[4] != FaceTextures[4])
            {
                OutDescription.FrontFaceTextureID.bChanged = true;
                OutDescription.FrontFaceTextureID.Value    = FaceTextures[4]->GetID();
            }

            if (BaseSkyboxResource->FaceTextures[5] != FaceTextures[5])
            {
                OutDescription.BackFaceTextureID.bChanged = true;
                OutDescription.BackFaceTextureID.Value    = FaceTextures[5]->GetID();
            }
        }
        else
        {
            OutDescription.RightFaceTextureID.Value    = FaceTextures[0]->GetID();
            OutDescription.RightFaceTextureID.bChanged = true;

            OutDescription.LeftFaceTextureID.Value    = FaceTextures[1]->GetID();
            OutDescription.LeftFaceTextureID.bChanged = true;

            OutDescription.TopFaceTextureID.Value    = FaceTextures[2]->GetID();
            OutDescription.TopFaceTextureID.bChanged = true;

            OutDescription.BottomFaceTextureID.Value    = FaceTextures[3]->GetID();
            OutDescription.BottomFaceTextureID.bChanged = true;

            OutDescription.FrontFaceTextureID.Value    = FaceTextures[4]->GetID();
            OutDescription.FrontFaceTextureID.bChanged = true;

            OutDescription.BackFaceTextureID.Value    = FaceTextures[5]->GetID();
            OutDescription.BackFaceTextureID.bChanged = true;
        }
    }

    IActor* CSkybox::CreateActorAsset(const FDString& InName) const
    {
        auto* ActorAsset = new CSkybox{ InName, nullptr, nullptr, nullptr, Width, Height, nullptr };
        for (u8 i = 0; i < 6; ++i)
        {
            ActorAsset->FaceTextures[i] = FaceTextures[i];
        }
        return ActorAsset;
    }

    CSkybox* CSkybox::CreateEmptyActorAsset(const FDString& InName)
    {
        auto* ActorAsset = new CSkybox{ InName, nullptr, nullptr, nullptr, 0, 0, nullptr };
        return ActorAsset;
    }

    void CSkybox::LoadAsset()
    {
        assert(ResourcePath.GetLength());

        FSkyboxDescription SkyboxDescription;
        if (ReadFromJSONFile(SkyboxDescription, *ResourcePath)) // @TODO strings from here don't get freed
        {
            FaceTextures[0] = GEngine.GetTexturesHolder().Get(SkyboxDescription.RightFaceTextureID.Value);
            FaceTextures[1] = GEngine.GetTexturesHolder().Get(SkyboxDescription.LeftFaceTextureID.Value);
            FaceTextures[2] = GEngine.GetTexturesHolder().Get(SkyboxDescription.TopFaceTextureID.Value);
            FaceTextures[3] = GEngine.GetTexturesHolder().Get(SkyboxDescription.BottomFaceTextureID.Value);
            FaceTextures[4] = GEngine.GetTexturesHolder().Get(SkyboxDescription.FrontFaceTextureID.Value);
            FaceTextures[5] = GEngine.GetTexturesHolder().Get(SkyboxDescription.BackFaceTextureID.Value);

            for (u8 i = 0; i < 6; ++i)
            {
                if (FaceTextures[i])
                {
                    FaceTextures[i]->Acquire(false, true);
                }
            }
        }
        else
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to load asset %s - couldn't read file %s", *Name, *ResourcePath)
        }
    }

    void CSkybox::UnloadAsset()
    {
        for (u8 i = 0; i < 6; ++i)
        {
            if (FaceTextures[i])
            {
                FaceTextures[i]->Release();
            }
        }
    }

    IActor* CSkybox::CreateActorInstance(CWorld* InWorld, const glm::vec3& InSpawnPosition)
    {
        auto* SpawnedSkybox = new CSkybox{ Name.GetCopy(), nullptr, InWorld, SkyboxCubemap, Width, Height, FaceTextures };
        World->SetSkybox(SpawnedSkybox);
        SpawnedSkybox->BaseSkyboxResource = this;
        return SpawnedSkybox;
    }

    void CSkybox::OnAddToWorld(CWorld* InWorld)
    {
        IActor::OnAddToWorld(InWorld);
        LoadAsset();
        if (!SkyboxCubemap)
        {
            const void* FacesData[6];
            for (u8 i = 0; i < 6; ++i)
            {
                FacesData[i] = FaceTextures[i]->TextureData;
            }
            SkyboxCubemap = gpu::CreateCubemap(Width,
                                               Width,
                                               gpu::ETextureDataFormat::SRGB,
                                               gpu::ETexturePixelFormat::RGB,
                                               gpu::ETextureDataType::UNSIGNED_BYTE,
                                               FacesData,
                                               Name,
                                               gpu::EMinTextureFilter::NEAREST,
                                               gpu::EMagTextureFilter::NEAREST,
                                               gpu::EWrapTextureFilter::CLAMP_TO_EDGE,
                                               gpu::EWrapTextureFilter::CLAMP_TO_EDGE,
                                               gpu::EWrapTextureFilter::CLAMP_TO_EDGE,
                                               { 0, 0, 0, 0 });
        }
    }
    void CSkybox::OnRemoveFromWorld(const bool& InbHardRemove)
    {
        IActor::OnRemoveFromWorld(InbHardRemove);
        UnloadAsset();

        if (InbHardRemove)
        {
            SkyboxCubemap->Free();
            delete SkyboxCubemap;
            SkyboxCubemap = nullptr;
        }
    }
} // namespace lucid::scene