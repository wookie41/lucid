#pragma once

#include "common/strings.hpp"
#include "stb_ds.h"
#include <type_traits>
#include <cassert>

namespace lucid::resources
{
    // Interface that represents a resource whose data can be stored in main/video memory or both
    class Resource
    {
      public:
        virtual void FreeMainMemory() = 0;
        virtual void FreeVideoMemory() = 0;
        virtual ~Resource() = default;

      protected:
        bool isVideoMemoryFreed = false;
        bool isMainMemoryFreed = false;
    };

    template <typename R, typename = std::enable_if<std::is_base_of<Resource, R>::value>>
    class ResourcesHolder
    {
      public:
        // DefaultResource is returned if the holder doesn't contain the resource
        ResourcesHolder(R* DefaultResource = nullptr) : defaultResource(DefaultResource) {}

        inline void SetDefaultResource(R* Resource) { defaultResource = Resource; };
        inline R* GetDefaultResource() { return defaultResource; };

        void Add(char const* ResourceName, R* Resource)
        {
            assert(shgeti(resourcesHashMap, ResourceName) == -1);
            shput(resourcesHashMap, ResourceName, Resource);
        }

        // returns nullptr if the resource doesn't exist
        R* Get(char const* ResourceName)
        {
            int resourceIndex = shgeti(resourcesHashMap, ResourceName);
            if (resourceIndex == -1)
            {
                return defaultResource;
            }
            return shget(resourcesHashMap, ResourceName);
        }

        // Removes the loaded resource and calls FreeResource() on it
        void Free(char const* ResourceName)
        {
            if (shgeti(resourcesHashMap, ResourceName) != -1)
            {
                R* res = shget(resourcesHashMap, ResourceName);
                res->FreeMainMemory();
                res->FreeVideoMemroy();
                shdel(resourcesHashMap, ResourceName);
            }
        }

        bool Contains(char const* ResourceName) { return shgeti(resourcesHashMap, ResourceName) != -1; }

        // Calls FreeResource() on all loaded resources and empties the holder
        void FreeAll()
        {
            for (u32 idx = 0; idx < shlen(resourcesHashMap); ++idx)
            {
                resourcesHashMap[idx]->FreeMainMemory();
                resourcesHashMap[idx]->FreeVideoMemory();
            }
            shfree(resourcesHashMap);
        }

      private:
        R* defaultResource = nullptr;
        struct
        {
            char* key;
            R* value;
        }* resourcesHashMap = NULL;
    };
} // namespace lucid::resources