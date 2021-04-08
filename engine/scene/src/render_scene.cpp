#include "scene/render_scene.hpp"
#include "scene/actors/skybox.hpp"

#include "devices/gpu/cubemap.hpp"

#include "resources/texture.hpp"

namespace lucid::scene
{
    CSkybox* CreateSkybox(const u32& RenderableId, const resources::CTextureResource* FaceTextures[6], const FANSIString& InName)
    {
        gpu::CCubemap* SkyboxCubemap = gpu::CreateCubemap({FaceTextures[0]->Width, FaceTextures[1]->Height},
                                                          gpu::ETextureDataFormat::SRGB,
                                                          gpu::ETexturePixelFormat::RGB,
                                                          gpu::ETextureDataType::UNSIGNED_BYTE,
                                                          FaceTextures,
                                                          InName);

        // @TODO skybox name is not freed
        return new CSkybox{CopyToString(*InName, InName.GetLength()), nullptr, SkyboxCubemap};
    }
}; // namespace lucid::scene
