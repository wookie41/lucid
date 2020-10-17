#pragma once

#include "common/strings.hpp"
#include "stb_ds.h"
#include <type_traits>
#include <cassert>

namespace lucid::resources
{
    class IResource
    {
      public:
        virtual void FreeResource() = 0;
        virtual ~IResource() = default;
    };

    template <typename R, typename = std::enable_if<std::is_base_of<IResource, R>::value>::type>
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
                res->FreeResource();
                shdel(resourcesHashMap, ResourceName);
            }
        }

        bool Contains(char const* ResourceName)
        {
            return shgeti(resourcesHashMap, ResourceName) != -1;
        }

        // Calls FreeResource() on all loaded resources and empties the holder
        void FreeAll()
        {
            for (uint32_t idx = 0; idx < shlen(resourcesHashMap); ++idx)
            {
                resourcesHashMap[idx]->FreeResource();
            }
            shfree(resourcesHashMap);
        }

      private:
        R* defaultResource = nullptr;
        struct { char *key; R* value; } *resourcesHashMap = NULL;
    };
} // namespace lucid::resources