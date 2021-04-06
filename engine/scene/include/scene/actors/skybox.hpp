#pragma once

#include "scene/actors/actor.hpp"

namespace lucid::gpu
{
    class CCubemap;
}; // namespace lucid::gpu


namespace lucid::scene
{
    class CSkybox : public IActor
    {
    public:
        explicit CSkybox(const FDString& InName, const IActor* InParent, gpu::CCubemap* InSkyboxCubemap)
        : IActor(InName, InParent), SkyboxCubemap(InSkyboxCubemap)
        {
            ActorType = EActorType::SKYBOX;
        }

        gpu::CCubemap* GetCubemap() const { return SkyboxCubemap; }

    private:
        gpu::CCubemap* SkyboxCubemap = nullptr;
    };

}