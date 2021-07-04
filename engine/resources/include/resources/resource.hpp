#pragma once

#include <devices/gpu/texture.hpp>

#include "common/strings.hpp"

namespace lucid::gpu
{
    class CTexture;
}

namespace lucid::resources
{
#define RESOURCE_NAME_LENGTH_SIZE (sizeof(u32))
#define RESOURCE_DATA_SIZE_SIZE (sizeof(u64))
#define RESOURCE_SERIALIZATION_VERSION_SIZE (sizeof(u32))
#define RESOURCE_FILE_HEADER_SIZE \
    (sizeof(lucid::UUID) + sizeof(EResourceType) + RESOURCE_SERIALIZATION_VERSION_SIZE + RESOURCE_DATA_SIZE_SIZE + RESOURCE_NAME_LENGTH_SIZE)

    enum EResourceType : u8
    {
        TEXTURE,
        MESH
    };

    /** Base class that represents a resource whose data can be stored in main/video memory or both */
    class CResource
    {
      public:
        CResource(const UUID&    InID,
                  const FString& InName,
                  const FString& InFilePath,
                  const u64&     InOffset,
                  const u64&     InDataSize,
                  const u32&     InAssetSerializationVersion);

        virtual EResourceType GetType() const = 0;

        /**
         * Called by LoadResource so the child class gets chance to load it's metadata,
         * e.x. the texture will read it's width and height in this method.
         */
        virtual void LoadMetadata(FILE* ResourceFile) = 0;

        virtual void LoadDataToMainMemorySynchronously() = 0;

        /**
         * Implementations are free to memory map the file if the data is not already loaded to the main memory
         * or just is the data loaded by the previous call to LoadDataToMainMemory*
         */
        virtual void LoadDataToVideoMemorySynchronously() = 0;

        virtual void SaveSynchronously(FILE* ResourceFile) const = 0;

        /** This method migrates the asset to it's latest version by filling the new values with defaults and saving it to the file */
        virtual void MigrateToLatestVersion() = 0;

        virtual void FreeMainMemory()  = 0;
        virtual void FreeVideoMemory() = 0;

        virtual void          LoadThumbnail(){};
        virtual void          MakeThumbnail(){};
        inline gpu::CTexture* GetThumbnail() const { return ThumbnailTexture; }

        inline const UUID&    GetID() const { return ID; }
        inline const FString& GetName() const { return Name; }
        inline const FString& GetFilePath() const { return FilePath; }
        inline u32            GetRefCount() const { return RefCount; }

        void Acquire(const bool& InbNeededInMainMemory, const bool& InbNeededInVideoMemory);
        void Release();

        virtual CResource* CreateCopy() const { return nullptr; } 
        
        /** Called from CResourceHolder */
        inline void MarkAsFreed() { RefCount = -1; }

        virtual ~CResource() = default;

      protected:
        void SaveHeader(FILE* ResourceFile) const;
        void Resave(const u32& InAssetSerializationVersion);

        UUID    ID;
        FString Name;

        /** Path the the file from which this resource was loaded */
        const FString FilePath;

        /** Offset at which the resource is stored in the file */
        u64 Offset;

        /** Size of the resource data in bytes, e.x. image data excluding the metadata like UUID, type etc */
        u64 DataSize;

        u32 AssetSerializationVersion = -1;

        bool bLoadedToMainMemory  = false;
        bool bLoadedToVideoMemory = false;

        bool IsVideoMemoryFreed = true;
        bool IsMainMemoryFreed  = true;

        u32 RefCount = 0;

        /** Useful in editor */
        gpu::CTexture* ThumbnailTexture = nullptr;
    };

    template <typename R, typename = std::enable_if<std::is_base_of<CResource, R>::value>>
    R* LoadResource(FILE* ResourceFile, const FString& ResourceFilePath);
} // namespace lucid::resources

#include "resources/resource.tpp"