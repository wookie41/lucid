#pragma once

#include <resources/resource.hpp>

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
                const u32& InHeight,
                const resources::CTextureResource* InFaceTextures[6]);


        virtual float GetVerticalMidPoint() const override;

#if DEVELOPMENT
        /** Editor stuff */
        virtual void UIDrawActorDetails() override;
#endif
        
        u32                                 Width, Height;
        gpu::CCubemap*                      SkyboxCubemap = nullptr;
        const resources::CTextureResource*  FaceTextures[6];
    };

    CSkybox* CreateSkybox(const resources::CTextureResource* FaceTextures[6],
                          const u32& InWidth,
                          const u32& InHeight,
                          const FString& InName);
} // namespace lucid::scene