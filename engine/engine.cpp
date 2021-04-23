#include "engine.hpp"

#include "stb_init.hpp"
#include "devices/gpu/init.hpp"
#include "devices/gpu/shaders_manager.hpp"
#include "schemas/types.hpp"
#include "schemas/json.hpp"

namespace lucid
{
    CEngine GEngine;
    EEngineInitError CEngine::InitEngine(const FEngineConfig& InEngineConfig)
    {
        srand(time(NULL));
        InitSTB();

        resources::InitTextures();

        // Init GPU
        if (gpu::Init({}) < 0)
        {
            return EEngineInitError::GPU_INIT_ERROR;
        }

        if (InEngineConfig.bHotReloadShaders)
        {
            ShadersManager.EnableHotReload();
        }
    }

    void CEngine::LoadResources()
    {
        // Read shaders databse
        FShadersDataBase ShadersDatabase;
        ReadFromJSONFile(ShadersDatabase, "assets/shaders/shaders_database.json");
        ShadersManager.LoadShadersDatabase(ShadersDatabase);
        
        // Read resource database
        ReadFromJSONFile(ResourceDatabase, "assets/resource_database.json");

        for (const FResourceDatabaseEntry& Entry : ResourceDatabase.Entries)
        {
            switch (Entry.Type)
            {
            case resources::MESH:
                {
                    if (resources::CMeshResource* LoadedMesh = resources::LoadMesh(Entry.Path))
                    {
                        LoadedMesh->LoadDataToMainMemorySynchronously();
                        LoadedMesh->LoadDataToVideoMemorySynchronously();
                        GEngine.GetMeshesHolder().Add(*Entry.Name, LoadedMesh);
                    }
                    break;
                }
            case resources::TEXTURE:
                {
                    if (resources::CTextureResource* LoadedTexture = resources::LoadTexture(Entry.Path))
                    {
                        LoadedTexture->LoadDataToMainMemorySynchronously();
                        LoadedTexture->LoadDataToVideoMemorySynchronously();
                        GEngine.GetTexturesHolder().Add(*Entry.Name, LoadedTexture);
                    }
                    break;
                }
            }
        }
    }

} // namespace lucid
