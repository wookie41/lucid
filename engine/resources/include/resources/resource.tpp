#pragma once

#include <common/log.hpp>

#include "resources/resource.hpp"
#include "resources/mesh_resource.hpp"
#include "resources/texture_resource.hpp"

namespace lucid::resources
{
    template <typename R, typename = std::enable_if<std::is_base_of<CResource, R>::value>>
    R* LoadResource(FILE* ResourceFile, const FString& ResourceFilePath)
    {
        const u64 Offset = ftell(ResourceFile);

        UUID ResourceUUID;
        EResourceType ResourceType;
        u64 ResourceDataSize;
        char* ResourceName;
        u32 ResourceNameLength;
        u32 AssetSerializationVersion;

        // Read the resource file header
        fread_s(&ResourceUUID, sizeof(ResourceUUID), sizeof(ResourceUUID), 1, ResourceFile);
        fread_s(&ResourceType, sizeof(ResourceType), sizeof(ResourceType), 1, ResourceFile);
        fread_s(&AssetSerializationVersion, sizeof(AssetSerializationVersion), sizeof(AssetSerializationVersion), 1, ResourceFile);
        fread_s(&ResourceDataSize, sizeof(ResourceDataSize), sizeof(ResourceDataSize), 1, ResourceFile);
        fread_s(&ResourceNameLength, sizeof(ResourceNameLength), sizeof(ResourceNameLength), 1, ResourceFile);

        ResourceName = (char*)malloc(ResourceNameLength + 1);
        ResourceName[ResourceNameLength] = '\0';

        fread_s(ResourceName, ResourceNameLength, ResourceNameLength, 1, ResourceFile);
        
        // Create a resource based on the type read from the resource file
        CResource* LoadedResource = nullptr;
        switch (ResourceType)
        {
        case TEXTURE:
        {
            LoadedResource = new CTextureResource{ ResourceUUID, FDString{ ResourceName }, ResourceFilePath, Offset, ResourceDataSize, AssetSerializationVersion };
            break;
        }
        case MESH:
        {
            LoadedResource = new CMeshResource{ ResourceUUID, FDString{ ResourceName }, ResourceFilePath, Offset, ResourceDataSize, AssetSerializationVersion };
            break;
        }
        default:
            assert(0);
        }

        assert(LoadedResource);
        LUCID_LOG(ELogLevel::INFO, "Imported asset %s with version %d", ResourceName, AssetSerializationVersion);

        LoadedResource->LoadMetadata(ResourceFile);

        fclose(ResourceFile);

        return (R*)LoadedResource;
    }
} // namespace lucid::resources
