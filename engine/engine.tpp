#pragma once

namespace lucid
{
    template <typename TActor, typename TActorDescription>
    TActor* CEngine::CreateActorInstance(scene::CWorld* InWorld, const TActorDescription& InActorDescription)
    {
        if (InActorDescription.BaseActorResourceId == sole::INVALID_UUID)
        {
            LUCID_LOG(ELogLevel::WARN, "Failed to create actor '%s'- base resource unknown", *InActorDescription.Name);
            return nullptr;
        }
        
        const auto ActorResourceId = InActorDescription.BaseActorResourceId;
        scene::IActor* BaseActorResource = nullptr;

        // Chcek if we've already loaded the actor resource info
        if (ActorResourceById.Contains(ActorResourceId))
        {
            BaseActorResource = ActorResourceById.Get(ActorResourceId);
        }
        else
        {
            // If not, check if we've got and entry for it and if so, load it
            if (ActorResourceInfoById.Contains(ActorResourceId))
            {
                FActorResourceInfo& ActorResourceInfo = ActorResourceInfoById.Get(ActorResourceId);
                assert(ActorResourceInfo.Type == TActor::GetActorTypeStatic());

                TActorDescription BaseActorResourceDescription;
                if(!ReadFromJSONFile(BaseActorResourceDescription, *ActorResourceInfo.ResourceFilePath))
                {
                    LUCID_LOG(ELogLevel::WARN, "Failed to create actor '%s' - couldn't load file '%s'", *InActorDescription.Name, *ActorResourceInfo.ResourceFilePath);
                    return nullptr;
                }
                
                BaseActorResource = TActor::CreateActor(nullptr, nullptr, BaseActorResourceDescription);

                ActorResourceById.Add(ActorResourceId, BaseActorResource);
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
