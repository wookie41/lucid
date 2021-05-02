#pragma once

#include "resources/resource.hpp"
#include "schemas/types.hpp"
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
                CWorld* InWorld,
                gpu::CCubemap* InSkyboxCubemap,
                const u32& InWidth,
                const u32& InHeight,
                const resources::CTextureResource* InFaceTextures[6]);

        virtual float GetVerticalMidPoint() const override;

        static  EActorType GetActorTypeStatic() { return EActorType::SKYBOX; }
        virtual EActorType GetActorType() const override  { return EActorType::SKYBOX; }
        static  CSkybox* CreateActor(CSkybox const* BaseActorResource, CWorld* InWorld, const FSkyboxDescription& InSkyboxDescription);

#if DEVELOPMENT
        /** Editor stuff */
        virtual void UIDrawActorDetails() override;

    protected:
        virtual void _SaveToResourceFile(const FString& InFilePath) override;
    public:
#endif
        void FillDescription(FSkyboxDescription& OutDescription) const;

        
        u32 Width, Height;
        gpu::CCubemap* SkyboxCubemap = nullptr;
        const resources::CTextureResource* FaceTextures[6];
        CSkybox const* BaseSkyboxResource;
    };

    CSkybox* CreateSkybox(const resources::CTextureResource* FaceTextures[6],
                          CWorld* InWorld,
                          const u32& InWidth,
                          const u32& InHeight,
                          const FString& InName);
} // namespace lucid::scene