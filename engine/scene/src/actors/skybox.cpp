#include "scene/actors/skybox.hpp"
#include "scene/world.hpp"

#include "devices/gpu/texture_enums.hpp"
#include "devices/gpu/cubemap.hpp"

#include "resources/texture_resource.hpp"
#include "engine/engine.hpp"
#include "schemas/json.hpp"

#include "imgui_lucid.h"

namespace lucid::scene
{
    CSkybox*
    CreateSkybox(resources::CTextureResource* InFaceTextures[6], CWorld* InWorld, const u32& InWidth, const u32& InHeight, const FString& InName)
    {
        gpu::CCubemap* SkyboxCubemap = gpu::CreateCubemap(InWidth,
                                                          InHeight,
                                                          gpu::ETextureDataFormat::SRGB,
                                                          gpu::ETexturePixelFormat::RGB,
                                                          gpu::ETextureDataType::UNSIGNED_BYTE,
                                                          InFaceTextures,
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
        bMovable    = false;
        bParentable = false;

        if (InFaceTextures)
        {
            for (u8 i = 0; i < 6; ++i)
            {
                FaceTextures[i] = InFaceTextures[i];
            }
        }
    }

    void CSkybox::UIDrawActorDetails()
    {
        IActor::UIDrawActorDetails();

        if (BaseActorAsset)
        {
            if (CSkybox* NewBaseSkybox = dynamic_cast<CSkybox*>(PrevBaseActorAsset))
            {
                BaseSkyboxResource = NewBaseSkybox;
                for (u8 i = 0; i < 6; ++i)
                {
                    FaceTextures[i] = BaseSkyboxResource->FaceTextures[i];
                }
            }
        }
        else
        {
            resources::CTextureResource* OldTextures[6];
            for (u8 i = 0; i < 6; ++i)
            {
                OldTextures[i] = FaceTextures[i];
            }

            ImGui::Text("Right");
            ImGuiTextureResourcePicker("Right face", &FaceTextures[0]);

            ImGui::Text("Left");
            ImGuiTextureResourcePicker("Left face", &FaceTextures[1]);

            ImGui::Text("Top");
            ImGuiTextureResourcePicker("Top face", &FaceTextures[2]);

            ImGui::Text("Bottom");
            ImGuiTextureResourcePicker("Bottom face", &FaceTextures[3]);

            ImGui::Text("Front");
            ImGuiTextureResourcePicker("Front face", &FaceTextures[4]);

            ImGui::Text("Back");
            ImGuiTextureResourcePicker("Back face", &FaceTextures[5]);

            auto ChildReference = &ChildReferences.Head;
            while (ChildReference && ChildReference->Element)
            {
                auto* ChildStaticMeshRef = ChildReference->Element;
                if (auto* SkyboxChildActor = dynamic_cast<CSkybox*>(ChildStaticMeshRef))
                {
                    for (u8 i = 0; i < 6; ++i)
                    {
                        if (SkyboxChildActor->FaceTextures[i] == OldTextures[i])
                        {
                            if (OldTextures[i])
                            {
                                OldTextures[i]->Release();
                            }

                            FaceTextures[i]->Acquire(false, true);
                            SkyboxChildActor->FaceTextures[i] = FaceTextures[i];
                        }
                    }
                }
                ChildReference = ChildReference->Next;
            }
        }
    }

    float CSkybox::GetVerticalMidPoint() const { return 0; }

    CSkybox* CSkybox::CreateActor(CSkybox* BaseActorResource, CWorld* InWorld, const FSkyboxDescription& InSkyboxDescription)
    {
        resources::CTextureResource* SkyboxFaces[6];

        if (InSkyboxDescription.RightFaceTextureID.bChanged)
        {
            SkyboxFaces[0] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.RightFaceTextureID.Value);
            SkyboxFaces[0]->Acquire(false, true);
        }
        else
        {
            SkyboxFaces[0] = BaseActorResource->FaceTextures[0];
        }

        if (InSkyboxDescription.LeftFaceTextureID.bChanged)
        {
            SkyboxFaces[1] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.LeftFaceTextureID.Value);
            SkyboxFaces[1]->Acquire(false, true);
        }
        else
        {
            SkyboxFaces[1] = BaseActorResource->FaceTextures[1];
        }

        if (InSkyboxDescription.TopFaceTextureID.bChanged)
        {
            SkyboxFaces[2] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.TopFaceTextureID.Value);
            SkyboxFaces[2]->Acquire(false, true);
        }
        else
        {
            SkyboxFaces[2] = BaseActorResource->FaceTextures[2];
        }

        if (InSkyboxDescription.BottomFaceTextureID.bChanged)
        {
            SkyboxFaces[3] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.BottomFaceTextureID.Value);
            SkyboxFaces[3]->Acquire(false, true);
        }
        else
        {
            SkyboxFaces[3] = BaseActorResource->FaceTextures[3];
        }

        if (InSkyboxDescription.FrontFaceTextureID.bChanged)
        {
            SkyboxFaces[4] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.FrontFaceTextureID.Value);
            SkyboxFaces[4]->Acquire(false, true);
        }
        else
        {
            SkyboxFaces[4] = BaseActorResource->FaceTextures[4];
        }

        if (InSkyboxDescription.BackFaceTextureID.bChanged)
        {
            SkyboxFaces[5] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.BackFaceTextureID.Value);
            SkyboxFaces[5]->Acquire(false, true);
        }
        else
        {
            SkyboxFaces[5] = BaseActorResource->FaceTextures[5];
        }

        auto* Skybox = CreateSkybox(SkyboxFaces, InWorld, SkyboxFaces[0]->Width, SkyboxFaces[0]->Height, "Skybox");
        InWorld->SetSkybox(Skybox);

        Skybox->Id     = InSkyboxDescription.Id;
        Skybox->Width  = BaseActorResource->Width;
        Skybox->Height = BaseActorResource->Height;

        Skybox->BaseActorAsset     = BaseActorResource;
        Skybox->BaseSkyboxResource = BaseActorResource;

        if (BaseActorResource)
        {
            BaseActorResource->ChildReferences.Add(Skybox);
        }

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
            OutDescription.Id = Id;

            // if (BaseSkyboxResource->FaceTextures[0] != FaceTextures[0])
            // {
            //     OutDescription.RightFaceTextureID.bChanged = true;
            //     OutDescription.RightFaceTextureID.Value    = FaceTextures[0]->GetID();
            // }
            //
            // if (BaseSkyboxResource->FaceTextures[1] != FaceTextures[1])
            // {
            //     OutDescription.LeftFaceTextureID.bChanged = true;
            //     OutDescription.LeftFaceTextureID.Value    = FaceTextures[1]->GetID();
            // }
            //
            // if (BaseSkyboxResource->FaceTextures[2] != FaceTextures[2])
            // {
            //     OutDescription.TopFaceTextureID.bChanged = true;
            //     OutDescription.TopFaceTextureID.Value    = FaceTextures[2]->GetID();
            // }
            //
            // if (BaseSkyboxResource->FaceTextures[3] != FaceTextures[3])
            // {
            //     OutDescription.BottomFaceTextureID.bChanged = true;
            //     OutDescription.BottomFaceTextureID.Value    = FaceTextures[3]->GetID();
            // }
            //
            // if (BaseSkyboxResource->FaceTextures[4] != FaceTextures[4])
            // {
            //     OutDescription.FrontFaceTextureID.bChanged = true;
            //     OutDescription.FrontFaceTextureID.Value    = FaceTextures[4]->GetID();
            // }
            //
            // if (BaseSkyboxResource->FaceTextures[5] != FaceTextures[5])
            // {
            //     OutDescription.BackFaceTextureID.bChanged = true;
            //     OutDescription.BackFaceTextureID.Value    = FaceTextures[5]->GetID();
            // }
        }
        else
        {
            OutDescription.RightFaceTextureID.Value    = FaceTextures && FaceTextures[0] ? FaceTextures[0]->GetID() : sole::INVALID_UUID;
            OutDescription.RightFaceTextureID.bChanged = true;

            OutDescription.LeftFaceTextureID.Value    = FaceTextures && FaceTextures[1] ? FaceTextures[1]->GetID() : sole::INVALID_UUID;
            OutDescription.LeftFaceTextureID.bChanged = true;

            OutDescription.TopFaceTextureID.Value    = FaceTextures && FaceTextures[2] ? FaceTextures[2]->GetID() : sole::INVALID_UUID;
            OutDescription.TopFaceTextureID.bChanged = true;

            OutDescription.BottomFaceTextureID.Value    = FaceTextures && FaceTextures[3] ? FaceTextures[3]->GetID() : sole::INVALID_UUID;
            OutDescription.BottomFaceTextureID.bChanged = true;

            OutDescription.FrontFaceTextureID.Value    = FaceTextures && FaceTextures[4] ? FaceTextures[4]->GetID() : sole::INVALID_UUID;
            OutDescription.FrontFaceTextureID.bChanged = true;

            OutDescription.BackFaceTextureID.Value    = FaceTextures && FaceTextures[5] ? FaceTextures[5]->GetID() : sole::INVALID_UUID;
            OutDescription.BackFaceTextureID.bChanged = true;
        }

        OutDescription.Name   = Name;
        OutDescription.Width  = Width;
        OutDescription.Height = Height;
    }

    IActor* CSkybox::CreateActorAsset(const FDString& InName) const
    {
        auto* ActorAsset = new CSkybox{ InName, nullptr, nullptr, nullptr, Width, Height, { nullptr } };
        for (u8 i = 0; i < 6; ++i)
        {
            ActorAsset->FaceTextures[i] = FaceTextures[i];
        }
        return ActorAsset;
    }

    CSkybox* CSkybox::LoadActorAsset(const FSkyboxDescription& InSkyboxDescription)
    {
        auto* ActorAsset            = new CSkybox{ InSkyboxDescription.Name, nullptr, nullptr, nullptr, 0, 0, nullptr };
        ActorAsset->FaceTextures[0] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.RightFaceTextureID.Value);
        ActorAsset->FaceTextures[1] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.LeftFaceTextureID.Value);
        ActorAsset->FaceTextures[2] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.TopFaceTextureID.Value);
        ActorAsset->FaceTextures[3] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.BottomFaceTextureID.Value);
        ActorAsset->FaceTextures[4] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.FrontFaceTextureID.Value);
        ActorAsset->FaceTextures[5] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.BackFaceTextureID.Value);
        ActorAsset->Width           = InSkyboxDescription.Width;
        ActorAsset->Width           = InSkyboxDescription.Height;
        return ActorAsset;
    }

    void CSkybox::LoadAsset()
    {
        assert(ResourcePath.GetLength());

        FSkyboxDescription SkyboxDescription;
        for (u8 i = 0; i < 6; ++i)
        {
            if (FaceTextures[i])
            {
                FaceTextures[i]->Acquire(false, true);
            }
        }

        bAssetLoaded = true;
    }

    void CSkybox::UnloadAsset()
    {
        if (!bAssetLoaded)
        {
            return;
        }
        for (u8 i = 0; i < 6; ++i)
        {
            if (FaceTextures[i])
            {
                FaceTextures[i]->Release();
            }
        }

        bAssetLoaded = false;
    }

    IActor* CSkybox::CreateActorInstance(CWorld* InWorld, const glm::vec3& InSpawnPosition)
    {
        if (!bAssetLoaded)
        {
            LoadAsset();
        }

        if (!bAssetLoaded)
        {
            return nullptr;
        }

        auto* SpawnedSkybox = CreateSkybox(FaceTextures, InWorld, Width, Height, Name.GetCopy());

        scene::CSkybox* CurrentSkybox = InWorld->GetSkybox();

        InWorld->SetSkybox(SpawnedSkybox);
        SpawnedSkybox->BaseSkyboxResource = this;

        if (CurrentSkybox)
        {
            delete CurrentSkybox;
        }

        return SpawnedSkybox;
    }

    void CSkybox::OnAddToWorld(CWorld* InWorld)
    {
        IActor::OnAddToWorld(InWorld);
        if (!SkyboxCubemap)
        {
            if (!BaseActorAsset->bAssetLoaded)
            {
                BaseActorAsset->LoadAsset();
            }
            
            SkyboxCubemap = gpu::CreateCubemap(Width,
                                               Width,
                                               gpu::ETextureDataFormat::SRGB,
                                               gpu::ETexturePixelFormat::RGB,
                                               gpu::ETextureDataType::UNSIGNED_BYTE,
                                               FaceTextures,
                                               Name,
                                               gpu::EMinTextureFilter::NEAREST,
                                               gpu::EMagTextureFilter::NEAREST,
                                               gpu::EWrapTextureFilter::CLAMP_TO_EDGE,
                                               gpu::EWrapTextureFilter::CLAMP_TO_EDGE,
                                               gpu::EWrapTextureFilter::CLAMP_TO_EDGE,
                                               { 0, 0, 0, 0 });
        }
        InWorld->SetSkybox(this);
    }
    void CSkybox::OnRemoveFromWorld(const bool& InbHardRemove)
    {
        IActor::OnRemoveFromWorld(InbHardRemove);
        if (InbHardRemove)
        {
            UnloadAsset();
            CleanupAfterRemove();
        }
    }

    void CSkybox::CleanupAfterRemove()
    {
        SkyboxCubemap->Free();
        delete SkyboxCubemap;
        SkyboxCubemap = nullptr;
    }

} // namespace lucid::scene