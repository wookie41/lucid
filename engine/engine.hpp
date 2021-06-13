#pragma once

#include <unordered_set>

#include "scene/material.hpp"
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
    } // namespace scene

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
        FDString ResourceFilePath;
        scene::EActorType Type;
    };

    class CEngine
    {

      public:
        EEngineInitError InitEngine(const FEngineConfig& InEngineConfig);
        
        void Shutdown();
        void LoadResources();

        inline FResourceDatabase&       GetResourceDatabase() { return ResourceDatabase; }
        inline FMaterialDatabase&       GetMaterialDatabase() { return MaterialDatabase; }
        inline CTexturesHolder&         GetTexturesHolder() { return TexturesHolder; }
        inline CMeshesHolder&           GetMeshesHolder() { return MeshesHolder; }
        inline CMaterialsHolder&        GetMaterialsHolder() { return MaterialsHolder; }
        inline scene::CRenderer*        GetRenderer() { return Renderer; }
        inline gpu::CShadersManager&    GetShadersManager() { return ShadersManager; }
        inline FActorDatabase&          GetActorsDatabase() { return ActorDatabase; }

        inline scene::CMaterial*    GetDefaultMaterial() { return DefaultMaterial; }
        void                        SetDefaultMaterial(scene::CMaterial* InMaterial);

        void                        SaveMaterialDatabase();

        inline FHashMap<UUID, scene::IActor*>& GetActorsResources() { return ActorResourceById; }

        void AddTextureResource(resources::CTextureResource* InTexture);
        void AddMeshResource(resources::CMeshResource* InMesh);

        void RemoveMeshResource(resources::CMeshResource* InMesh);
        void RemoveTextureResource(resources::CTextureResource*);

        void AddMaterialAsset(scene::CMaterial* InMaterial,const scene::EMaterialType& InMaterialType, const FDString& InMaterialPath);
        void RemoveMaterialAsset(scene::CMaterial* InMaterial);

        void AddActorAsset(scene::IActor* InActorResource);
        void RemoveActorAsset(scene::IActor* InActorResource);

        // @TODO There will be called internally by the engien
        void BeginFrame();

        // @TODO This doesn't guarantee that the frame has finished rendering
        void EndFrame();

        inline void AddEngineObject(IEngineObject* InEngineObject)
        {
            EngineObjects.Add(InEngineObject);
        }

        inline void RemoveEngineObject(IEngineObject* InEngineObject)
        {
            EngineObjects.Remove(InEngineObject);
        }

        inline void AddActorWithDirtyResources(scene::IActor* InActor) { ActorsWithDirtyResources.insert(InActor); }

    protected:

        template <typename TActor, typename TActorDescription>
        void LoadActorAsset(const FActorDatabaseEntry& Entry);

        scene::CRenderer* Renderer = nullptr;

        gpu::CShadersManager ShadersManager {};

        FResourceDatabase   ResourceDatabase {};
        FMaterialDatabase   MaterialDatabase {};
        FActorDatabase      ActorDatabase;

        CMeshesHolder    MeshesHolder {};
        CTexturesHolder  TexturesHolder {};
        CMaterialsHolder MaterialsHolder {};

        FHashMap<UUID, scene::IActor*>       ActorResourceById;

        scene::CMaterial* DefaultMaterial = nullptr;

        FLinkedList<IEngineObject> EngineObjects;

        std::unordered_set<scene::IActor*> ActorsWithDirtyResources;
#if DEVELOPMENT
    public:
        CActorThumbsGenerator* ThumbsGenerator = nullptr;
#endif
    };

    extern CEngine GEngine;
} // namespace lucid

#include "engine.tpp"