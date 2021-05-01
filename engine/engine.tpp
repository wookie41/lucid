#pragma once

namespace lucid
{
    template <typename TActor, typename TActorDescription>
    TActor* CEngine::CreateActorInstance(scene::CWorld* InWorld, const UUID& InActorResourceId, const TActorDescription& InActorDescription)
    {
        const auto ActorResourceIdStr = InActorResourceId.str();
        if (!ActorResourceById.Contains(ActorResourceIdStr.c_str()))
        {
            return nullptr;
        }

        if (ActorResourceById.Contains(ActorResourceIdStr.c_str()))
        {
            const FActorResourceInfo& ActorResourceInfo = ActorResourceById.Get(ActorResourceIdStr.c_str());
            assert(ActorResourceInfo.Type == TActor::GetActorTypeStatic());

            return TActor::CreateActor(InWorld, InActorDescription);
        }

        return nullptr;    
    }
}