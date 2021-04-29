#pragma once

#include "common/strings.hpp"
#include "stb_ds.h"
#include "resources/resource.hpp"

#include <type_traits>
#include <cassert>

namespace lucid::resources
{
    
    template <typename R, typename = std::enable_if<std::is_base_of<CResource, R>::value>>
    class CResourcesHolder
    {
        using FResourcesHashMap = struct
        {
            char* key;
            R* value;
        };

      public:
        
        /** DefaultResource is returned if the holder doesn't contain the resource */
        CResourcesHolder(R* DefaultResource = nullptr) : defaultResource(DefaultResource) {}

        inline void SetDefaultResource(R* Resource) { defaultResource = Resource; };
        inline R* GetDefaultResource() { return defaultResource; };

        void Add(char const* ResourceName, R* Resource)
        {
            assert(shgeti(ResourcesHashMap, ResourceName) == -1);
            shput(ResourcesHashMap, ResourceName, Resource);
        }

        void Remove(char const* ResourceName)
        {
            shdel(ResourcesHashMap, ResourceName);
        }

        /** Returns the default resource if the resource doesn't exist */
        R* Get(char const* ResourceName)
        {
            int resourceIndex = shgeti(ResourcesHashMap, ResourceName);
            if (resourceIndex == -1)
            {
                return defaultResource;
            }
            return shget(ResourcesHashMap, ResourceName);
        }

        /** Removes the loaded resource and calls FreeMainMemory() and FreeVideoMemory() on it */
        void Free(char const* ResourceName)
        {
            if (shgeti(ResourcesHashMap, ResourceName) != -1)
            {
                R* res = shget(ResourcesHashMap, ResourceName);
                res->FreeMainMemory();
                res->FreeVideoMemroy();
                shdel(ResourcesHashMap, ResourceName);
            }
        }

        bool Contains(char const* ResourceName) { return shgeti(ResourcesHashMap, ResourceName) != -1; }

        /** Calls FreeResource() on all loaded resources and empties the holder */
        void FreeAll()
        {
            for (u32 idx = 0; idx < shlen(ResourcesHashMap); ++idx)
            {
                ResourcesHashMap[idx]->FreeMainMemory();
                ResourcesHashMap[idx]->FreeVideoMemory();
            }
            shfree(ResourcesHashMap);
        }

        u32 Length() const { return shlen(ResourcesHashMap); }
        R* GetByIndex(const u32& Index) const
        {
            if(Index >= shlen(ResourcesHashMap))
            {
                return defaultResource;
            }
            return ResourcesHashMap[Index].value;
        }
        
        inline FResourcesHashMap* GetResourcesHashMap() const { return ResourcesHashMap; }
    
      private:
        R* defaultResource = nullptr;
        FResourcesHashMap* ResourcesHashMap = NULL;
    };
} // namespace lucid::resources