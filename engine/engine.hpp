#pragma once

#include "common/types.hpp"
#include "devices/gpu/shaders_manager.hpp"
#include "resources/resources_holder.hpp"
#include "resources/texture_resource.hpp"
#include "resources/mesh_resource.hpp"

namespace lucid
{
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
        
        inline resources::CResourcesHolder<resources::CTextureResource>& GetTexturesHolder() const { return TexturesHolder; } 
        inline resources::CResourcesHolder<resources::CTextureResource>& GetMeshesHolder() const { return TexturesHolder; } 

    protected:

        gpu::CShadersManager ShadersManager;
        
        resources::CResourcesHolder<resources::CTextureResource> TexturesHolder;
        resources::CResourcesHolder<resources::CTextureResource> MeshesHolder;
    };

    extern CEngine GEngine;
}
