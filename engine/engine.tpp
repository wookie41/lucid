#pragma once

#include "scene/actors/static_mesh.hpp"
#include "scene/actors/skybox.hpp"

namespace lucid
{
    template <typename TActor, typename TActorDescription>
    void CEngine::LoadActorAsset(const FActorDatabaseEntry& ActorEntry)
    {
        TActorDescription ActorAssetDescription;
        if (!ReadFromJSONFile(ActorAssetDescription, *ActorEntry.ActorPath)) //@TODO strings don't get freed
        {
            LUCID_LOG(ELogLevel::WARN,
                      "Failed to create actor '%s' - couldn't load file '%s'",
                      ActorEntry.ActorId.str().c_str(),
                      *ActorEntry.ActorPath);
            return;
        }

        auto* ActorAsset = TActor::LoadAsset(ActorAssetDescription);
        ActorAsset->ResourceId = ActorEntry.ActorId;
        ActorAsset->ResourcePath = ActorEntry.ActorPath;

        ActorResourceById.Add(ActorEntry.ActorId, ActorAsset);
    }
} // namespace lucid
