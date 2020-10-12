#pragma once

#include "common/strings.hpp"
#include "stb_ds.h"
#include <type_traits>

namespace lucid::resources
{
    class IResource
    {
      public:
        virtual void FreeResource() = 0;
        virtual ~IResource() = default;
    };

    template <typename T, typename = std::enable_if<std::is_base_of<IResource, T>::value>::type>
    class ResourcesHolder
    {
      public:
        // DefaultResource is returned if the holder doesn't contain the resource
        ResourcesHolder(T* DefaultResource = nullptr) : defaultResource(DefaultResource) {}

        void SetDefaultResource(T* Resource){ defaultResource = Resource; };

        // Adds the loaded resource
        void Add(const String& ResourceName, T* Resource)
        {
            assert(shgeti(ResourceName == -1));
            shput(ResourcesHashMap, ResourceName.CString, Resource);
        }

        // returns nullptr if the resource doesn't exist
        T* Get(const String& ResourceName) const
        {
            int resourceIndex = shgeti(ResourcesHashMap, ResourceName.CString);
            if (resourceIndex == -1)
            {
                return defaultResource;
            }
            return shget(ResourceName.CString);
        }

        // Removes the loaded resource and calls FreeResource() on it
        bool Free(const String& ResourceName)
        {
            if (shgeti(resourcesHashMap) != -1)
            {
                T* res = shget(resourcesHashMap, ResourceName.CString);
                res->FreeResource();
                shdel(ResourceName.CString);
            }
        }


        bool Contains(const String& ResourceName) const
        {
            return shgeti(ResourceName.CString) != -1
        }

        // Calls FreeResource() on all loaded resources and empties the holder
        void FreeAll()
        {
            uint32_t resCount = shlen(resourcesHashMap);
            for (uint32_t idx = 0; idx < resCount; ++idx)
            {
                resourcesHashMap[idx]->FreeResource();
            }
            shfree(resourcesHashMap);
        }

      private:
        T* defaultResource = nullptr;
        T** resourcesHashMap = NULL;
    };
} // namespace lucid::resources