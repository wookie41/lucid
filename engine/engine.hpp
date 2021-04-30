#pragma once

#include "common/types.hpp"

#include "resources/resource.hpp"
#include "resources/resources_holder.hpp"
#include "resources/texture_resource.hpp"
#include "resources/mesh_resource.hpp"
#include "devices/gpu/shaders_manager.hpp"

#include "schemas/types.hpp"

namespace lucid
{
    namespace scene
    {
        class CRenderer;
    }
    
    using CTexturesHolder = resources::CResourcesHolder<resources::CTextureResource>;
    using CMeshesHolder = resources::CResourcesHolder<resources::CMeshResource>;
    using CMaterialsHolder = FStringHashMap<scene::CMaterial*>;

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
    
    class CEngine
    {
        
    public:

        EEngineInitError InitEngine(const FEngineConfig& InEngineConfig);
        void             LoadResources();
        
        inline FResourceDatabase&       GetResourceDatabase()   { return ResourceDatabase; }
        inline CTexturesHolder&         GetTexturesHolder()     { return TexturesHolder; } 
        inline CMeshesHolder&           GetMeshesHolder()       { return MeshesHolder; }
        inline CMaterialsHolder&        GetMaterialsHolder()    { return MaterialsHolder; }
        inline scene::CRenderer*        GetRenderer()           { return Renderer; }
        inline gpu::CShadersManager&    GetShadersManager()     { return ShadersManager; }

        void AddTextureResource(resources::CTextureResource* InTexture,  const FString& InSourcePath);
        void AddMeshResource(resources::CMeshResource* InMesh, const FString& InSourcePath);

        void RemoveMeshResource(resources::CMeshResource* InMesh);
        void RemoveTextureResource(resources::CTextureResource*);
    
    protected:

        scene::CRenderer* Renderer = nullptr;

        gpu::CShadersManager ShadersManager;

        FResourceDatabase ResourceDatabase;
        FMaterialDatabase MaterialDatabase;

        CMeshesHolder    MeshesHolder;
        CTexturesHolder  TexturesHolder;
        CMaterialsHolder MaterialsHolder;

#if DEVELOPMENT
    public:
        CActorThumbsGenerator* ThumbsGenerator;
#endif

    };

    extern CEngine GEngine;
}
