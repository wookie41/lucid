#pragma once

#include "resources/resources_holder.hpp"
#include "common/bytes.hpp"

#include "devices/gpu/vao.hpp"
#include <cstdint>

namespace lucid::resources
{
    class CTextureResource;

    class CMeshResource : public CResource
    {
      public:
        CMeshResource(const UUID& InID,
                         const FString& InName,
                         const FString& InFilePath,
                         const u64& InOffset,
                         const u64& InDataSize,
                         const u32& InAssetSerializationVersion);

        virtual EResourceType GetType() const override { return MESH; };

        virtual void LoadMetadata(FILE* ResourceFile) override;
        virtual void LoadDataToMainMemorySynchronously() override;
        virtual void LoadDataToVideoMemorySynchronously() override;

        virtual void SaveSynchronously(FILE* ResourceFile) override;
        
        virtual void FreeMainMemory() override;
        virtual void FreeVideoMemory() override;

        u32 VertexCount     = 0;
        u32 ElementCount    = 0;
        
        gpu::CVertexArray*    VAO           = nullptr;
        gpu::CBuffer*         VertexBuffer  = nullptr;
        gpu::CBuffer*         ElementBuffer = nullptr;

        FMemBuffer            VertexData;
        FMemBuffer            ElementData;
    };

    /**
     *  Loads the mesh from the given directory, requires that
     *  textures are stored in the same directory as the model file
     */
    CMeshResource* ImportMesh(const FString& MeshFilePath, const FString& MeshName);

    CMeshResource* LoadMesh(const FString& FilePath);    
} // namespace lucid::resources
