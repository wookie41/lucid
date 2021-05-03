#pragma once

#include <scene/material.hpp>

#include "scene/actors/actor.hpp"

#include "common/types.hpp"

#include "schemas/types.hpp"

#include "resources/resource.hpp"
#include "resources/resources_holder.hpp"
#include "resources/texture_resource.hpp"
#include "resources/mesh_resource.hpp"
#include "devices/gpu/shaders_manager.hpp"

namespace lucid
{
    namespace scene
    {
        class IActor;
        class CRenderer;
        class CStaticMesh;
        class CFlatMaterial;
        class CBlinnPhongMaterial;
        class CBlinnPhongMapsMaterial;
    }

#define DECLARE_LOAD_MATERIAL_FUNC(Suffix, TMaterialDescription, TMaterial) void Load##Suffix(TDYNAMICARRAY<FMaterialDatabaseEntry> Entries);

    using CTexturesHolder = resources::CResourcesHolder<resources::CTextureResource>;
    using CMeshesHolder = resources::CResourcesHolder<resources::CMeshResource>;
    using CMaterialsHolder = FHashMap<UUID, scene::CMaterial*>;
    using CStaticMeshesHolder = FHashMap<UUID, scene::CStaticMesh*>;

    class CActorThumbsGenerator;

    enum class EEngineInitError : u8
    {
        NONE,
        GPU_INIT_ERROR,
    };

    struct FEngineConfig
    {
        bool bHotReloadShaders;
    };

    struct FActorResourceInfo
    {
        FDString            ResourceFilePath;
        scene::EActorType   Type;
    };
    
    class CEngine
    {
        
    public:

        EEngineInitError InitEngine(const FEngineConfig& InEngineConfig);
        void             Shutdown();
        void             LoadResources();
        
        inline FResourceDatabase&       GetResourceDatabase()   { return ResourceDatabase; }
        inline FMaterialDatabase&       GetMaterialDatabase()   { return MaterialDatabase; }
        inline CTexturesHolder&         GetTexturesHolder()     { return TexturesHolder; } 
        inline CMeshesHolder&           GetMeshesHolder()       { return MeshesHolder; }
        inline CMaterialsHolder&        GetMaterialsHolder()    { return MaterialsHolder; }
        inline scene::CRenderer*        GetRenderer()           { return Renderer; }
        inline gpu::CShadersManager&    GetShadersManager()     { return ShadersManager; }
        inline FActorDatabase&          GetActorsDatabase()     { return ActorDatabase; }
        inline scene::CMaterial*        GetDefaultMaterial()    { return DefaultMaterial; }

        void AddTextureResource(resources::CTextureResource* InTexture,  const FString& InSourcePath);
        void AddMeshResource(resources::CMeshResource* InMesh, const FString& InSourcePath);

        void RemoveMeshResource(resources::CMeshResource* InMesh);
        void RemoveTextureResource(resources::CTextureResource*);

        template <typename TActor, typename TActorDescription>
        TActor* CreateActorInstance(scene::CWorld* InWorld, const TActorDescription& InActorDescription);
    
    protected:

        DECLARE_LOAD_MATERIAL_FUNC(FlatMaterials, FFlatMaterialDescription, scene::CFlatMaterial)
        DECLARE_LOAD_MATERIAL_FUNC(BlinnPhongMaterials, FBlinnPhongMaterialDescription, scene::CBlinnPhongMaterial)
        DECLARE_LOAD_MATERIAL_FUNC(BlinnPhongMapsMaterials, FBlinnPhongMapsMaterialDescription, scene::CBlinnPhongMapsMaterial)
        
        scene::CRenderer* Renderer = nullptr;

        gpu::CShadersManager ShadersManager {};

        FResourceDatabase   ResourceDatabase {};
        FMaterialDatabase   MaterialDatabase {};
        FActorDatabase      ActorDatabase;

        CMeshesHolder    MeshesHolder {};
        CTexturesHolder  TexturesHolder {};
        CMaterialsHolder MaterialsHolder {};

        FHashMap<UUID, FActorResourceInfo>   ActorResourceInfoById;        
        FHashMap<UUID, scene::IActor*>       ActorResourceById;

        scene::CMaterial* DefaultMaterial = nullptr;

#if DEVELOPMENT
    public:
        CActorThumbsGenerator* ThumbsGenerator = nullptr;
#endif

    };

    extern CEngine GEngine;
}

#include "engine.tpp"