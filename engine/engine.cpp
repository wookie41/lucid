#include "engine/engine.hpp"

#include "stb_init.hpp"
#include "devices/gpu/init.hpp"
#include "devices/gpu/shaders_manager.hpp"
#include "misc/actor_thumbs.hpp"

#include "scene/blinn_phong_material.hpp"
#include "scene/flat_material.hpp"
#include "scene/forward_renderer.hpp"

#define LUCID_SCHEMAS_IMPLEMENTATION
#include "schemas/types.hpp"
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

        // Setup the renderer
        auto* ForwardRenderer = new scene::CForwardRenderer{ 32, 4 };
        ForwardRenderer->AmbientStrength = 0.05;
        ForwardRenderer->NumSamplesPCF = 20;
        ForwardRenderer->ResultResolution = { 1920, 1080 };
        Renderer = ForwardRenderer;
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

        // Load resources data
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

        // Load materials database
        ReadFromJSONFile(MaterialDatabase, "assets/materials_database.json");

        // Create materials for the materials definitions
        for (const FFlatMaterialDescription& MaterialDescription : MaterialDatabase.FlatMaterials)
        {
            scene::FlatMaterial* FlatMaterial = scene::CreateFlatMaterial(MaterialDescription);
            MaterialsHolder.Add(*FlatMaterial->GetName(), FlatMaterial);
        }

        for (const FBlinnPhongMaterialDescription& MaterialDescription : MaterialDatabase.BlinnPhongMaterials)
        {
            scene::CBlinnPhongMaterial* BlinnPhongMaterial = scene::CreateBlinnPhongMaterial(MaterialDescription);
            MaterialsHolder.Add(*BlinnPhongMaterial->GetName(), BlinnPhongMaterial);
        }

        for (const FBlinnPhongMapsMaterialDescription& MaterialDescription : MaterialDatabase.BlinnPhongMapsMaterials)
        {
            scene::CBlinnPhongMapsMaterial* BlinnPhongMapsMaterial = scene::CreateBlinnPhongMapsMaterial(MaterialDescription);
            MaterialsHolder.Add(*BlinnPhongMapsMaterial->GetName(), BlinnPhongMapsMaterial);
        }

        Renderer->Setup();
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

    void CEngine::RemoveTextureResource(resources::CTextureResource* InTexture)
    {
        // @TODO Don't allow to delete resources referenced by other resources + free main/video memory
        ResourceDatabase.Entries.erase(std::remove_if(
          ResourceDatabase.Entries.begin(),
          ResourceDatabase.Entries.end(),
          [&](const FResourceDatabaseEntry& Entry) { return Entry.Id == InTexture->GetID(); }));

        
        MeshesHolder.Remove(*InTexture->GetName());
        remove(*InTexture->GetFilePath());

        WriteToJSONFile(ResourceDatabase, "assets/resource_database.json");
    }
    
    void CEngine::RemoveMeshResource(resources::CMeshResource* InMesh)
    {
        // @TODO Don't allow to delete resources referenced by other resources + free main/video memory
        ResourceDatabase.Entries.erase(std::remove_if(
          ResourceDatabase.Entries.begin(),
          ResourceDatabase.Entries.end(),
          [&](const FResourceDatabaseEntry& Entry) { return Entry.Id == InMesh->GetID(); }));

        
        MeshesHolder.Remove(*InMesh->GetName());
        remove(*InMesh->GetFilePath());

        WriteToJSONFile(ResourceDatabase, "assets/resource_database.json");
    }
} // namespace lucid
