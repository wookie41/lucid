#include "scene/actors/skybox.hpp"

#include "engine/engine.hpp"
#include "devices/gpu/cubemap.hpp"
#include "resources/texture_resource.hpp"
#include "schemas/json.hpp"
#include "scene/world.hpp"

namespace lucid::scene
{
    CSkybox* CreateSkybox(const resources::CTextureResource* InFaceTextures[6],
                          CWorld* InWorld,
                          const u32& InWidth,
                          const u32& InHeight,
                          const FString& InName)
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
                                                          InName);

        return new CSkybox{ CopyToString(*InName, InName.GetLength()), nullptr, InWorld, SkyboxCubemap, InWidth, InHeight, InFaceTextures };
    }

    CSkybox::CSkybox(const FDString& InName,
                     const IActor* InParent,
                     CWorld* InWorld,
                     gpu::CCubemap* InSkyboxCubemap,
                     const u32& InWidth,
                     const u32& InHeight,
                     const resources::CTextureResource* InFaceTextures[6])
    : IActor(InName, InParent, InWorld), SkyboxCubemap(InSkyboxCubemap), Width(InWidth), Height(InHeight)
    {
        for (u8 i = 0; i < 6; ++i)
        {
            FaceTextures[i] = InFaceTextures[i];
        }
    }

    void CSkybox::UIDrawActorDetails()
    {
        
    }
    
    float CSkybox::GetVerticalMidPoint() const
    {
        return 0;
    }

    CSkybox* CSkybox::CreateActor(CSkybox const* BaseActorResource, CWorld* InWorld, const FSkyboxDescription& InSkyboxDescription)
    {
        const resources::CTextureResource* SkyboxFaces[6];
        if (InSkyboxDescription.RightFaceTexture.bChanged)
        {
            SkyboxFaces[0] = BaseActorResource->FaceTextures[0];
        }
        else
        {
            SkyboxFaces[0] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.RightFaceTexture.Value);
        }

        if (InSkyboxDescription.RightFaceTexture.bChanged)
        {
            SkyboxFaces[1] = BaseActorResource->FaceTextures[0];
        }
        else
        {
            SkyboxFaces[1] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.LeftFaceTexture.Value);
        }


        if (InSkyboxDescription.TopFaceTexture.bChanged)
        {
            SkyboxFaces[2] = BaseActorResource->FaceTextures[2];
        }
        else
        {
            SkyboxFaces[2] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.TopFaceTexture.Value);
        }


        if (InSkyboxDescription.BottomFaceTexture.bChanged)
        {
            SkyboxFaces[3] = BaseActorResource->FaceTextures[3];
        }
        else
        {
            SkyboxFaces[3] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.BottomFaceTexture.Value);
        }


        if (InSkyboxDescription.FrontFaceTexture.bChanged)
        {
            SkyboxFaces[4] = BaseActorResource->FaceTextures[4];
        }
        else
        {
            SkyboxFaces[4] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.FrontFaceTexture.Value);
        }

        if (InSkyboxDescription.BackFaceTexture.bChanged)
        {
            SkyboxFaces[5] = BaseActorResource->FaceTextures[5];
        }
        else
        {
            SkyboxFaces[5] = GEngine.GetTexturesHolder().Get(InSkyboxDescription.BackFaceTexture.Value);
        }
        
        auto* Skybox = CreateSkybox(SkyboxFaces, InWorld, SkyboxFaces[0]->Width, SkyboxFaces[0]->Height, "Skybox");
        if (InWorld)
        {
            InWorld->SetSkybox(Skybox);            
        }
        Skybox->BaseSkyboxResource = BaseActorResource;;
        return Skybox;   
    }

    void CSkybox::_SaveToResourceFile(const FString& InFilePath)
    {
        FSkyboxDescription SkyboxDescription;
        FillDescription(SkyboxDescription);
        WriteToJSONFile(SkyboxDescription, *InFilePath);
    }

    void CSkybox::FillDescription(FSkyboxDescription& OutDescription) const
    {
        if (BaseSkyboxResource)
        {
            OutDescription.BaseActorResourceId = BaseSkyboxResource->ResourceId;

            if (BaseSkyboxResource->FaceTextures[0] != FaceTextures[0])
            {
                OutDescription.RightFaceTexture.bChanged = true;
                OutDescription.RightFaceTexture.Value    = FaceTextures[0]->GetID();
            }

            if (BaseSkyboxResource->FaceTextures[1] != FaceTextures[1])
            {
                OutDescription.LeftFaceTexture.bChanged = true;
                OutDescription.LeftFaceTexture.Value    = FaceTextures[1]->GetID();
            }
            
            if (BaseSkyboxResource->FaceTextures[2] != FaceTextures[2])
            {
                OutDescription.TopFaceTexture.bChanged = true;
                OutDescription.TopFaceTexture.Value    = FaceTextures[2]->GetID();
            }
            
            if (BaseSkyboxResource->FaceTextures[3] != FaceTextures[3])
            {
                OutDescription.BottomFaceTexture.bChanged = true;
                OutDescription.BottomFaceTexture.Value    = FaceTextures[3]->GetID();
            }
            
            if (BaseSkyboxResource->FaceTextures[4] != FaceTextures[4])
            {
                OutDescription.FrontFaceTexture.bChanged = true;
                OutDescription.FrontFaceTexture.Value    = FaceTextures[4]->GetID();
            }
            
            if (BaseSkyboxResource->FaceTextures[5] != FaceTextures[5])
            {
                OutDescription.BackFaceTexture.bChanged = true;
                OutDescription.BackFaceTexture.Value    = FaceTextures[5]->GetID();
            }
        }
        else
        {
            OutDescription.LeftFaceTexture.Value    = FaceTextures[0]->GetID();
            OutDescription.RightFaceTexture.Value   = FaceTextures[1]->GetID();
            OutDescription.TopFaceTexture.Value     = FaceTextures[2]->GetID();
            OutDescription.BottomFaceTexture.Value  = FaceTextures[3]->GetID();
            OutDescription.FrontFaceTexture.Value   = FaceTextures[4]->GetID();
            OutDescription.BackFaceTexture.Value    = FaceTextures[5]->GetID();
        }
    }

} // namespace lucid::scene