#pragma once

#include "scene/actors/actor.hpp"

namespace lucid::gpu
{
    class CCubemap;
    class CTexture;
}; // namespace lucid::gpu

namespace lucid::scene
{
    class CSkybox : public IActor
    {
      public:
        CSkybox(const FDString& InName,
                const IActor* InParent,
                gpu::CCubemap* InSkyboxCubemap,
                const u32& InWidth,
                const u32& InHeight);


#if DEVELOPMENT
        /** Editor stuff */
        virtual void UIDrawSceneHierarchy() override;
#endif
        
        u32 Width, Height;
        gpu::CCubemap* SkyboxCubemap = nullptr;
    };

    CSkybox* CreateSkybox(const void* FaceTexturesDAta[6],
                          const u32& InWidth,
                          const u32& InHeight,
                          const FString& InName);
} // namespace lucid::scene