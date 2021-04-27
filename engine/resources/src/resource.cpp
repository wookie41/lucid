﻿#include "resources/resource.hpp"

#include "common/log.hpp"
#include "resources/texture_resource.hpp"

namespace lucid::resources
{
    CResource::CResource(const UUID& InID,
                         const FString& InName,
                         const FString& InFilePath,
                         const u64& InOffset,
                         const u64& InDataSize,
                         const u32& InAssetSerializationVersion)
    : ID(InID), Name(InName), FilePath(InFilePath), Offset(InOffset), DataSize(InDataSize), AssetSerializationVersion(InAssetSerializationVersion)
    {
    }

    void CResource::SaveHeader(FILE* ResourceFile) const
    {
        const u32           NameLength      = Name.GetLength();
        const EResourceType ResourceType    = GetType();

        fwrite(&ID, sizeof(ID), 1, ResourceFile);
        fwrite(&ResourceType, sizeof(ResourceType), 1, ResourceFile);
        fwrite(&AssetSerializationVersion, sizeof(AssetSerializationVersion), 1, ResourceFile);
        fwrite(&DataSize, sizeof(DataSize), 1, ResourceFile);
        fwrite(&NameLength, sizeof(NameLength), 1, ResourceFile);
        fwrite(*Name, Name.GetLength(), 1, ResourceFile);
    }

    void CResource::Resave() const
    {
        FILE* ResourceFile;
        if(fopen_s(&ResourceFile, *FilePath, "wb+") == 0)
        {
            SaveSynchronously(ResourceFile);
            fclose(ResourceFile);
        }
        else
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to resave resource %s - couldn't open file", *Name);
        }
    }    
} // namespace lucid::resources
