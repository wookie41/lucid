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

    CSkybox* CSkybox::CreateActor(CWorld* InWorld, const FSkyboxDescription& InSkyboxDescription)
    {
        const resources::CTextureResource* SkyboxFaces[6] {
            GEngine.GetTexturesHolder().Get(*InSkyboxDescription.FacesTextures[0]), GEngine.GetTexturesHolder().Get(*InSkyboxDescription.FacesTextures[1]),
            GEngine.GetTexturesHolder().Get(*InSkyboxDescription.FacesTextures[2]),   GEngine.GetTexturesHolder().Get(*InSkyboxDescription.FacesTextures[3]),
            GEngine.GetTexturesHolder().Get(*InSkyboxDescription.FacesTextures[4]), GEngine.GetTexturesHolder().Get(*InSkyboxDescription.FacesTextures[5])
        };

        auto* Skybox = CreateSkybox(SkyboxFaces, InWorld, SkyboxFaces[0]->Width, SkyboxFaces[0]->Height, "Skybox");
        InWorld->SetSkybox(Skybox);
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
        for (u8 i = 0; i < 6; ++i)
        {
            OutDescription.FacesTextures[i] = FDString { *FaceTextures[i]->GetName() };             
        }
    }

} // namespace lucid::scene