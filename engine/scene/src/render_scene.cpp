#include "scene/render_scene.hpp"
#include "scene/actors/skybox.hpp"

#include "devices/gpu/cubemap.hpp"

#include "resources/texture.hpp"

namespace lucid::scene
{
    CSkybox* CreateSkybox(const u32& RenderableId, const FArray<FString>& InSkyboxFacesPaths, const FANSIString& InName)
    {
        const char* skyboxFacesData[6];
        resources::CTextureResource* textureResources[6];

        for (u8 face = 0; face < 6; ++face)
        {
            textureResources[face] =
                resources::LoadJPEG(*InSkyboxFacesPaths[face], true, gpu::ETextureDataType::UNSIGNED_BYTE, false, false,
                                    InName);
            assert(textureResources[face]);
            skyboxFacesData[face] = (char*)textureResources[face]->TextureData;
        }

        gpu::CCubemap* skyboxCubemap = gpu::CreateCubemap({textureResources[0]->Width, textureResources[1]->Height},
                                                          gpu::ETextureDataFormat::SRGB,
                                                          gpu::ETexturePixelFormat::RGB,
                                                          gpu::ETextureDataType::UNSIGNED_BYTE,
                                                          skyboxFacesData,
                                                          InName);

        for (u8 face = 0; face < 6; ++face)
        {
            textureResources[face]->FreeMainMemory();
        }

        // @TODO skybox name is not freed
        return new CSkybox{CopyToString(*InName, InName.GetLength()), nullptr, skyboxCubemap};
    }
}; // namespace lucid::scene
