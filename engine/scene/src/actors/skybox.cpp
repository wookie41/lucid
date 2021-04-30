#include "scene/actors/skybox.hpp"

#include "devices/gpu/cubemap.hpp"
#include "resources/texture_resource.hpp"

namespace lucid::scene
{
    CSkybox* CreateSkybox(const resources::CTextureResource* InFaceTextures[6],
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

        return new CSkybox{ CopyToString(*InName, InName.GetLength()), nullptr, SkyboxCubemap, InWidth, InHeight, InFaceTextures };
    }

    CSkybox::CSkybox(const FDString& InName,
                     const IActor* InParent,
                     gpu::CCubemap* InSkyboxCubemap,
                     const u32& InWidth,
                     const u32& InHeight,
                     const resources::CTextureResource* InFaceTextures[6])
    : IActor(InName, InParent), SkyboxCubemap(InSkyboxCubemap), Width(InWidth), Height(InHeight)
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
} // namespace lucid::scene