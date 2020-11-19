#include "scene/render_scene.hpp"
#include "devices/gpu/cubemap.hpp"
#include "resources/texture.hpp"

namespace lucid::scene
{
    Skybox CreateSkybox(const char* FacesPaths[6])
    {
        const char* skyboxFacesData[6];
        resources::TextureResource* textureResources[6];

        for (uint8_t face = 0; face < 6; ++face)
        {
            textureResources[face] =
              resources::LoadJPEG(FacesPaths[face], true, gpu::TextureDataType::UNSIGNED_BYTE, false, false);
            assert(textureResources[face]);
            skyboxFacesData[face] = (char*)textureResources[face]->TextureData;
        }

        gpu::Cubemap* skyboxCubemap =
          gpu::CreateCubemap({ textureResources[0]->Width, textureResources[1]->Height }, gpu::TextureFormat::SRGB,
                             gpu::TextureFormat::RGB, gpu::TextureDataType::UNSIGNED_BYTE, skyboxFacesData);

        for (uint8_t face = 0; face < 6; ++face)
        {
            textureResources[face]->FreeMainMemory();
        }

        return { skyboxCubemap };
    }
}; // namespace lucid::scene