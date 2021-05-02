#pragma once

namespace lucid
{
    template <typename TActor, typename TActorDescription>
    TActor* CEngine::CreateActorInstance(scene::CWorld* InWorld, const UUID& InActorResourceId, const TActorDescription& InActorDescription)
    {
        if (InActorDescription.BaseActorResourceId != sole::INVALID_UUID)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to create actor '%s' - base resource unknown");
            return nullptr;
        }
        const auto ActorResourceIdStr = InActorResourceId.str();

        scene::IActor* BaseActorResource = nullptr;

        // Chcek if we've already loaded the actor resource info
        if (ActorResourceById.Contains(ActorResourceIdStr.c_str()))
        {
            BaseActorResource = ActorResourceById.Get(ActorResourceIdStr.c_str());
        }
        else
        {
            // If not, check if we've got and entry for it and if so, load it
            if (ActorResourceInfoById.Contains(ActorResourceIdStr.c_str()))
            {
                FActorResourceInfo& ActorResourceInfo = ActorResourceInfoById.Get(ActorResourceIdStr.c_str());
                assert(ActorResourceInfo.Type == TActor::GetActorTypeStatic());

                TActorDescription ActorResourceDescription;
                if(!ReadFromJSONFile(ActorResourceDescription, *ActorResourceInfo.ResourceFilePath))
                {
                    LUCID_LOG(ELogLevel::WARN, "Failed to create actor '%s' - couldn't load file", *ActorResourceInfo.ResourceFilePath);
                    return nullptr;
                }
                
                BaseActorResource = TActor::CreateActor(nullptr, nullptr, ActorResourceDescription);
                BaseActorResource->ResourceId = InActorResourceId;

                ActorResourceById.Add(ActorResourceIdStr.c_str(), BaseActorResource);
            }
            else
            {
                // Fail otherwise
                LUCID_LOG(ELogLevel::WARN, "Failed to create actor '%s' - no base resource info", *InActorDescription.Name);
                return nullptr;
            }
        }

        return TActor::CreateActor((TActor*)BaseActorResource, InWorld, InActorDescription);
    }
}
