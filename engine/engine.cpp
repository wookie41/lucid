#include "engine/engine.hpp"

#include "stb_init.hpp"
#include "devices/gpu/init.hpp"
#include "devices/gpu/shaders_manager.hpp"
#include "misc/actor_thumbs.hpp"

#define LUCID_SCHEMAS_IMPLEMENTATION
#include "schemas/binary.hpp"
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

        ThumbsGenerator = new CActorThumbsGenerator;
        ThumbsGenerator->Setup();
        
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
                        LoadedMesh->Thumb = ThumbsGenerator->GenerateMeshThumb(256, 256, LoadedMesh);
                        GEngine.GetMeshesHolder().Add(*Entry.Name, LoadedMesh);

                        if (Entry.bIsDefault)
                        {
                            GEngine.GetMeshesHolder().SetDefaultResource(LoadedMesh);
                        }
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

                        if (Entry.bIsDefault)
                        {
                            GEngine.GetTexturesHolder().SetDefaultResource(LoadedTexture);
                        }
                        
                    }
                    break;
                }
            }
        }
    }

    void CEngine::AddTextureResource(resources::CTextureResource* InTexture, const FString& InSourcePath)
    {
        FResourceDatabaseEntry Entry;
        Entry.Id = InTexture->GetID();
        Entry.Name = CopyToString(*InTexture->GetName(), InTexture->GetName().GetLength());
        Entry.Path = CopyToString(*InTexture->GetFilePath(), InTexture->GetFilePath().GetLength());
        Entry.Type = resources::TEXTURE;
        Entry.bIsDefault = false;
        ResourceDatabase.Entries.push_back(Entry);
        TexturesHolder.Add(*InTexture->GetName(), InTexture);
        WriteToJSONFile(GEngine.GetResourceDatabase(), "assets/resource_database.json");
    }

    void CEngine::AddMeshResource(resources::CMeshResource* InMesh, const FString& InSourcePath)
    {
        FResourceDatabaseEntry Entry;
        Entry.Id = InMesh->GetID();
        Entry.Name = CopyToString(*InMesh->GetName(), InMesh->GetName().GetLength());
        Entry.Path = CopyToString(*InMesh->GetFilePath(), InMesh->GetFilePath().GetLength());
        Entry.Type = resources::MESH;
        Entry.bIsDefault = false;
        ResourceDatabase.Entries.push_back(Entry);
        MeshesHolder.Add(*InMesh->GetName(), InMesh);
        WriteToJSONFile(GEngine.GetResourceDatabase(), "assets/resource_database.json");
    }


} // namespace lucid
