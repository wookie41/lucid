#pragma once

#include "common/strings.hpp"
#include "resources/resource.hpp"
#include "common/collections.hpp"

#include <type_traits>
#include <cassert>

namespace lucid::resources
{
    
    template <typename R, typename = std::enable_if<std::is_base_of<CResource, R>::value>>
    class CResourcesHolder
    {
      public:
        
        /** DefaultResource is returned if the holder doesn't contain the resource */
        CResourcesHolder(R* InDefaultResource = nullptr) : DefaultResource(InDefaultResource) {}

        inline void SetDefaultResource(R* InDefaultResource) { DefaultResource = InDefaultResource; };
        inline R* GetDefaultResource() { return DefaultResource; };

        inline void Add(const UUID& InId, R* Resource)
        {
            ResourcesHashMap.Add(InId, Resource);
        }

        inline void Remove(const UUID& InId)
        {
            ResourcesHashMap.Remove(InId);
        }

        /** Returns the default resource if the resource doesn't exist */
        inline R* Get(const UUID& InId)
        {
            R* Resource = ResourcesHashMap.Get(InId);
            return Resource ? Resource : DefaultResource;
        }

        /** Removes the loaded resource and calls FreeMainMemory() and FreeVideoMemory() on it */
        inline void Free(const UUID& InId)
        {
            R* Resource= ResourcesHashMap.Get(InId);
            if (Resource)
            {
                Resource->FreeMainMemory();
                Resource->FreeVideoMemroy();
                ResourcesHashMap.Remove(InId);
            }
        }

        inline bool Contains(const UUID& InResourceId) { return ResourcesHashMap.Contains(InResourceId); }

        /** Calls FreeResource() on all loaded resources and empties the holder */
        inline void FreeAll()
        {
            for (u32 idx = 0; idx < ResourcesHashMap.GetLength(); ++idx)
            {
                ResourcesHashMap.Get(idx)->FreeMainMemory();
                ResourcesHashMap.Get(idx)->FreeVideoMemory();
            }
            ResourcesHashMap.FreeAll();
        }

        inline u32 Length() const { return ResourcesHashMap.GetLength(); }
        inline R* GetByIndex(const u32& Index)
        {
            return ResourcesHashMap.Get(Index);
        }
        
        inline FHashMap<UUID, R*>& GetResourcesHashMap() const { return ResourcesHashMap; }
    
      private:
        R*                      DefaultResource = nullptr;
        FHashMap<UUID, R*>      ResourcesHashMap;
    };
} // namespace lucid::resources