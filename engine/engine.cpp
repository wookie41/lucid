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
#define DEFINE_LOAD_MATERIAL_FUNC(Suffix, TMaterialDescription, TMaterial)                                \
    void CEngine::Load##Suffix(TDYNAMICARRAY<FMaterialDatabaseEntry> Entries)                             \
    {                                                                                                     \
        bool bSuccess = false;                                                                            \
        for (FMaterialDatabaseEntry Entry : Entries)                                                      \
        {                                                                                                 \
            TMaterialDescription MaterialDescription;                                                     \
            switch (Entry.FileFormat)                                                                     \
            {                                                                                             \
            case EFileFormat::Json:                                                                       \
            {                                                                                             \
                bSuccess = ReadFromJSONFile(MaterialDescription, *Entry.MaterialPath);                    \
                break;                                                                                    \
            }                                                                                             \
            case EFileFormat::Binary:                                                                     \
            {                                                                                             \
                bSuccess = ReadFromBinaryFile(MaterialDescription, *Entry.MaterialPath);                  \
                break;                                                                                    \
            }                                                                                             \
            }                                                                                             \
            if (bSuccess)                                                                                 \
            {                                                                                             \
                TMaterial* Material = TMaterial::CreateMaterial(MaterialDescription, Entry.MaterialPath); \
                MaterialsHolder.Add(Material->GetID(), Material);                                         \
                if (Entry.bDefault)                                                                       \
                {                                                                                         \
                    DefaultMaterial = Material;                                                           \
                }                                                                                         \
            }                                                                                             \
            else                                                                                          \
            {                                                                                             \
                LUCID_LOG(ELogLevel::WARN, "Failed to load material from path %s", *Entry.MaterialPath);  \
            }                                                                                             \
        }                                                                                                 \
    }

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
        ForwardRenderer->ResultResolution = { 1280, 720 };
        Renderer = ForwardRenderer;
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
                    LoadedMesh->LoadDataToMainMemorySynchronously();
                    LoadedMesh->LoadDataToVideoMemorySynchronously();
                    LoadedMesh->Thumb = ThumbsGenerator->GenerateMeshThumb(256, 256, LoadedMesh);
                    GEngine.GetMeshesHolder().Add(Entry.Id, LoadedMesh);

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
                    GEngine.GetTexturesHolder().Add(Entry.Id, LoadedTexture);

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
        LoadFlatMaterials(MaterialDatabase.FlatMaterials);
        LoadBlinnPhongMaterials(MaterialDatabase.BlinnPhongMaterials);
        LoadBlinnPhongMapsMaterials(MaterialDatabase.BlinnPhongMapsMaterials);

        // Load actor database
        ReadFromJSONFile(ActorDatabase, "assets/databases/actors.json");
        for (const FActorDatabaseEntry& Entry : ActorDatabase.Entries)
        {
            FActorResourceInfo ActorResourceInfo;
            ActorResourceInfo.Type = Entry.ActorType;
            ActorResourceInfo.ResourceFilePath = Entry.ActorPath;
            ActorResourceInfoById.Add(Entry.ActorId, ActorResourceInfo);
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
        TexturesHolder.Add(InTexture->GetID(), InTexture);
        WriteToJSONFile(GEngine.GetResourceDatabase(), "assets/databases/resources.json");
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

    DEFINE_LOAD_MATERIAL_FUNC(FlatMaterials, FFlatMaterialDescription, scene::CFlatMaterial)
    DEFINE_LOAD_MATERIAL_FUNC(BlinnPhongMaterials, FBlinnPhongMaterialDescription, scene::CBlinnPhongMaterial)
    DEFINE_LOAD_MATERIAL_FUNC(BlinnPhongMapsMaterials, FBlinnPhongMapsMaterialDescription, scene::CBlinnPhongMapsMaterial)

    void CEngine::Shutdown()
    {
        MaterialDatabase.FlatMaterials.clear();
        MaterialDatabase.BlinnPhongMaterials.clear();
        MaterialDatabase.BlinnPhongMapsMaterials.clear();

        for (u32 i = 0; i < MaterialsHolder.GetLength(); ++i)
        {
            MaterialsHolder.GetByIndex(i)->SaveToResourceFile(EFileFormat::Json);
        }

        WriteToJSONFile(MaterialDatabase, "assets/databases/materials.json");

        ActorDatabase.Entries.clear();

        for (u32 i = 0; i < ActorResourceById.GetLength(); ++i)
        {
            ActorResourceById.GetByIndex(i)->SaveToResourceFile();
        }
        
        WriteToJSONFile(ActorDatabase, "assets/databases/actors.json");
    }
} // namespace lucid
