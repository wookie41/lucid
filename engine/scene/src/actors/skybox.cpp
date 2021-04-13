#include "scene/actors/skybox.hpp"
#include "devices/gpu/cubemap.hpp"
#include "resources/texture.hpp"

namespace lucid::scene
{
    CSkybox* CreateSkybox(const void* FaceTexturesData[6],
                          const u32& InWidth,
                          const u32& InHeight,
                          const FString& InName)
    {
        gpu::CCubemap* SkyboxCubemap = gpu::CreateCubemap(InWidth,
                                                          InHeight,
                                                          gpu::ETextureDataFormat::SRGB,
                                                          gpu::ETexturePixelFormat::RGB,
                                                          gpu::ETextureDataType::UNSIGNED_BYTE,
                                                          FaceTexturesData,
                                                          InName);

        return new CSkybox{ CopyToString(*InName, InName.GetLength()), nullptr, SkyboxCubemap, InWidth, InHeight };
    }

    CSkybox::CSkybox(const FDString& InName,
                     const IActor* InParent,
                     gpu::CCubemap* InSkyboxCubemap,
                     const u32& InWidth,
                     const u32& InHeight)
    : IActor(InName, InParent), SkyboxCubemap(InSkyboxCubemap), Width(InWidth), Height(InHeight)
    {
    }

} // namespace lucid::scene