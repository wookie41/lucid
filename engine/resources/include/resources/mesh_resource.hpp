#pragma once

#include "resources/resources_holder.hpp"
#include "common/bytes.hpp"
#include "common/collections.hpp"

#include "devices/gpu/vao.hpp"

namespace lucid::resources
{
    class CTextureResource;

    struct FSubMesh
    {
        gpu::CVertexArray*  VAO = nullptr;;
        gpu::CBuffer*       VertexBuffer  = nullptr;
        gpu::CBuffer*       ElementBuffer = nullptr;

        bool bHasPositions;
        bool bHasNormals;
        bool bHasTangetns;
        bool bHasUVs;

        u32 VertexCount = 0;
        u32 ElementCount = 0;
        
        FMemBuffer VertexDataBuffer;
        FMemBuffer ElementDataBuffer;
    };
    
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

        virtual void SaveSynchronously(FILE* ResourceFile) const override;

        virtual void MigrateToLatestVersion() override;
        
        virtual void FreeMainMemory() override;
        virtual void FreeVideoMemory() override;

        float MinPosX = 0, MaxPosX = 0;
        float MinPosY = 0, MaxPosY = 0;
        float MinPosZ = 0, MaxPosZ = 0;

        FArray<FSubMesh> SubMeshes { 1, true };
        
#if DEVELOPMENT
        /** Thumb texture used to show the mesh in asset browser */
        gpu::CTexture*  Thumb;
#endif
    };

    /**
     *  Loads the mesh from the given directory, requires that
     *  textures are stored in the same directory as the model file
     */
    CMeshResource* ImportMesh(const FString& MeshFilePath, const FString& InMeshFilePath, const FString& MeshName);

    CMeshResource* LoadMesh(const FString& FilePath);    
} // namespace lucid::resources
