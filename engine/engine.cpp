#include "engine/engine.hpp"

#include "stb_init.hpp"
#include "devices/gpu/init.hpp"
#include "devices/gpu/shaders_manager.hpp"
#include "misc/actor_thumbs.hpp"

#include "scene/blinn_phong_material.hpp"
#include "scene/flat_material.hpp"
#include "scene/forward_renderer.hpp"

#define LUCID_SCHEMAS_IMPLEMENTATION
#include "scene/material.hpp"
#include "scene/actors/static_mesh.hpp"

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
        ForwardRenderer->NumSamplesPCF = 25;
        ForwardRenderer->ResultResolution = { 1920, 1080 };
        Renderer = ForwardRenderer;

        EngineObjects.Add(&MeshesHolder);
        EngineObjects.Add(&TexturesHolder);
    }

    void CEngine::LoadResources()
    {
        // Read shaders databse
        FShadersDataBase ShadersDatabase;
        ReadFromJSONFile(ShadersDatabase, "assets/databases/shaders.json");
        ShadersManager.LoadShadersDatabase(ShadersDatabase);

        ThumbsGenerator = new CActorThumbsGenerator;
        ThumbsGenerator->Setup();

        // Read resource database
        ReadFromJSONFile(ResourceDatabase, "assets/databases/resources.json");

        // Load resources data
        for (const FResourceDatabaseEntry& Entry : ResourceDatabase.Entries)
        {
            switch (Entry.Type)
            {
            case resources::MESH:
            {
                if (resources::CMeshResource* LoadedMesh = resources::LoadMesh(Entry.Path))
                {
                    GEngine.GetMeshesHolder().Add(Entry.Id, LoadedMesh);

                    LoadedMesh->LoadThumbnail();
                    if (!LoadedMesh->GetThumbnail())
                    {
                        LoadedMesh->MakeThumbnail();
                    }

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
                    GEngine.GetTexturesHolder().Add(Entry.Id, LoadedTexture);

                    LoadedTexture->LoadThumbnail();
                    if (!LoadedTexture->GetThumbnail())
                    {
                        LoadedTexture->MakeThumbnail();
                    }
                    
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
        ReadFromJSONFile(MaterialDatabase, "assets/databases/materials.json");

        // Create materials for the materials definitions
        bool bFileReadSuccess = false;
        scene::CMaterial* LoadedMaterial = nullptr;
        for (const FMaterialDatabaseEntry& Entry : MaterialDatabase.Entries)
        {
            LoadedMaterial = nullptr;

            switch (Entry.MaterialType)
            {
            case scene::EMaterialType::FLAT:
            {
                FFlatMaterialDescription MaterialDescription;
                switch (Entry.FileFormat)
                {
                case EFileFormat::Json:
                {
                    bFileReadSuccess = ReadFromJSONFile(MaterialDescription, *Entry.MaterialPath);
                    break;
                }
                case EFileFormat::Binary:
                {
                    bFileReadSuccess = ReadFromBinaryFile(MaterialDescription, *Entry.MaterialPath);
                    break;
                }
                }
                if (bFileReadSuccess)
                {
                    LoadedMaterial = scene::CFlatMaterial::CreateMaterial(MaterialDescription, Entry.MaterialPath);
                }
                else
                {
                    LUCID_LOG(ELogLevel::WARN, "Failed to load material from path %s", *Entry.MaterialPath);
                }

                break;
            }

            case scene::EMaterialType::BLINN_PHONG:
            {
                FBlinnPhongMaterialDescription MaterialDescription;
                switch (Entry.FileFormat)
                {
                case EFileFormat::Json:
                {
                    bFileReadSuccess = ReadFromJSONFile(MaterialDescription, *Entry.MaterialPath);
                    break;
                }
                case EFileFormat::Binary:
                {
                    bFileReadSuccess = ReadFromBinaryFile(MaterialDescription, *Entry.MaterialPath);
                    break;
                }
                }
                if (bFileReadSuccess)
                {
                    LoadedMaterial = scene::CBlinnPhongMaterial::CreateMaterial(MaterialDescription, Entry.MaterialPath);
                }
                else
                {
                    LUCID_LOG(ELogLevel::WARN, "Failed to load material from path %s", *Entry.MaterialPath);
                }

                break;
            }
            case scene::EMaterialType::BLINN_PHONG_MAPS:
            {
                FBlinnPhongMapsMaterialDescription MaterialDescription;
                switch (Entry.FileFormat)
                {
                case EFileFormat::Json:
                {
                    bFileReadSuccess = ReadFromJSONFile(MaterialDescription, *Entry.MaterialPath);
                    break;
                }
                case EFileFormat::Binary:
                {
                    bFileReadSuccess = ReadFromBinaryFile(MaterialDescription, *Entry.MaterialPath);
                    break;
                }
                }
                if (bFileReadSuccess)
                {
                    LoadedMaterial = scene::CBlinnPhongMapsMaterial::CreateMaterial(MaterialDescription, Entry.MaterialPath);
                }
                else
                {
                    LUCID_LOG(ELogLevel::WARN, "Failed to load material from path %s", *Entry.MaterialPath);
                }

                break;
            }
            default:
                assert(0);
            }

            if (LoadedMaterial)
            {
                LoadedMaterial->bIsAsset = true;
                MaterialsHolder.Add(LoadedMaterial->GetID(), LoadedMaterial);
                if (Entry.bDefault)
                {
                    DefaultMaterial = LoadedMaterial;
                }
            }
        }

        // Load actor database
        ReadFromJSONFile(ActorDatabase, "assets/databases/actors.json");
        for (const FActorDatabaseEntry& Entry : ActorDatabase.Entries)
        {
            switch (Entry.ActorType)
            {
            case scene::EActorType::STATIC_MESH:
            {
                LoadActorAsset<scene::CStaticMesh, FStaticMeshDescription>(Entry);
                break;
            }
            case scene::EActorType::SKYBOX:
            {
                LoadActorAsset<scene::CSkybox, FSkyboxDescription>(Entry);
                break;
            }
            default:
                LUCID_LOG(ELogLevel::WARN, "Unsupported actor type %d", Entry.ActorType);
            }
        }

        Renderer->Setup();
    }

    void CEngine::AddTextureResource(resources::CTextureResource* InTexture)
    {
        FResourceDatabaseEntry Entry;
        Entry.Id = InTexture->GetID();
        Entry.Name = CopyToString(*InTexture->GetName(), InTexture->GetName().GetLength());
        Entry.Path = CopyToString(*InTexture->GetFilePath(), InTexture->GetFilePath().GetLength());
        Entry.Type = resources::TEXTURE;
        Entry.bIsDefault = false;
        ResourceDatabase.Entries.push_back(Entry);
        TexturesHolder.Add(InTexture->GetID(), InTexture);
        WriteToJSONFile(ResourceDatabase, "assets/databases/resources.json");
    }

    void CEngine::AddMeshResource(resources::CMeshResource* InMesh)
    {
        FResourceDatabaseEntry Entry;
        Entry.Id = InMesh->GetID();
        Entry.Name = CopyToString(*InMesh->GetName(), InMesh->GetName().GetLength());
        Entry.Path = CopyToString(*InMesh->GetFilePath(), InMesh->GetFilePath().GetLength());
        Entry.Type = resources::MESH;
        Entry.bIsDefault = false;
        ResourceDatabase.Entries.push_back(Entry);
        MeshesHolder.Add(InMesh->GetID(), InMesh);
        WriteToJSONFile(GEngine.GetResourceDatabase(), "assets/databases/resources.json");
    }

    void CEngine::RemoveTextureResource(resources::CTextureResource* InTexture)
    {
        // @TODO Don't allow to delete resources referenced by other resources + free main/video memory
        ResourceDatabase.Entries.erase(std::remove_if(
          ResourceDatabase.Entries.begin(), ResourceDatabase.Entries.end(), [&](const FResourceDatabaseEntry& Entry) {
              return Entry.Id == InTexture->GetID();
          }));

        TexturesHolder.Remove(InTexture->GetID());
        remove(*InTexture->GetFilePath());

        WriteToJSONFile(ResourceDatabase, "assets/databases/resources.json");
    }

    void CEngine::RemoveMeshResource(resources::CMeshResource* InMesh)
    {
        // @TODO Don't allow to delete resources referenced by other resources + free main/video memory
        ResourceDatabase.Entries.erase(std::remove_if(
          ResourceDatabase.Entries.begin(), ResourceDatabase.Entries.end(), [&](const FResourceDatabaseEntry& Entry) {
              return Entry.Id == InMesh->GetID();
          }));

        MeshesHolder.Remove(InMesh->GetID());
        remove(*InMesh->GetFilePath());

        WriteToJSONFile(ResourceDatabase, "assets/databases/resources.json");
    }

    void CEngine::Shutdown() {}

    void CEngine::AddMaterialAsset(scene::CMaterial* InMaterial,
                                   const scene::EMaterialType& InMaterialType,
                                   const FDString& InMaterialPath)
    {
        MaterialsHolder.Add(InMaterial->GetID(), InMaterial);
        MaterialDatabase.Entries.push_back({ InMaterial->GetID(), InMaterialPath, EFileFormat::Json, InMaterialType, false });
        SaveMaterialDatabase();
    }

    void CEngine::RemoveMaterialAsset(scene::CMaterial* InMaterial)
    {
        MaterialDatabase.Entries.erase(std::remove_if(
          MaterialDatabase.Entries.begin(), MaterialDatabase.Entries.end(), [&](const FMaterialDatabaseEntry& Entry) {
              if (Entry.Id == InMaterial->GetID())
              {
                  remove(*Entry.MaterialPath);
                  return true;
              }
              return false;
          }));

        MaterialsHolder.Remove(InMaterial->GetID());
        SaveMaterialDatabase();
    }

    void CEngine::RemoveActorAsset(scene::IActor* InActorResource)
    {
        ActorDatabase.Entries.erase(
          std::remove_if(ActorDatabase.Entries.begin(), ActorDatabase.Entries.end(), [&](const FActorDatabaseEntry& Entry) {
              if (Entry.ActorId == InActorResource->ResourceId)
              {
                  remove(*Entry.ActorPath);
                  return true;
              }
              return false;
          }));

        ActorResourceById.Remove(InActorResource->ResourceId);
        WriteToJSONFile(ActorDatabase, "assets/databases/actors.json");
    }

    void CEngine::AddActorAsset(scene::IActor* InActorResource)
    {
        ActorDatabase.Entries.push_back({ InActorResource->ResourceId, InActorResource->ResourcePath, InActorResource->GetActorType() });
        ActorResourceById.Add(InActorResource->ResourceId, InActorResource);
        WriteToJSONFile(ActorDatabase, "assets/databases/actors.json");
    }

    void CEngine::SetDefaultMaterial(scene::CMaterial* InMaterial)
    {
        bool bDefaultSet = false;
        bool bDefaultUnset = false;

        for (FMaterialDatabaseEntry& Entry : MaterialDatabase.Entries)
        {
            if (Entry.Id == InMaterial->GetID())
            {
                Entry.bDefault = true;
                bDefaultSet = true;
                if (bDefaultUnset || !DefaultMaterial)
                {
                    break;
                }
            }
            else if (DefaultMaterial && DefaultMaterial->GetID() == Entry.Id)
            {
                Entry.bDefault = false;
                bDefaultUnset = true;
                if (bDefaultSet)
                {
                    break;
                }
            }
        }

        DefaultMaterial = InMaterial;
    }

    void CEngine::SaveMaterialDatabase()
    {
        WriteToJSONFile(MaterialDatabase, "assets/databases/materials.json");
    }

    void CEngine::BeginFrame()
    {
        gpu::QueryDeviceStatus();

        auto Node = &EngineObjects.Head;
        while (Node && Node->Element)
        {
            Node->Element->OnFrameBegin();
            Node = Node->Next;
        }
    }

    void CEngine::EndFrame()
    {
        auto Node = &EngineObjects.Head;
        while (Node && Node->Element)
        {
            Node->Element->OnFrameEnd();
            Node = Node->Next;
        }
    }
} // namespace lucid
