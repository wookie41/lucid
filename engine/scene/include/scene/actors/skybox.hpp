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
                IActor* InParent,
                CWorld* InWorld,
                gpu::CCubemap* InSkyboxCubemap,
                const u32& InWidth,
                const u32& InHeight,
                const resources::CTextureResource* InFaceTextures[6]);

        virtual float GetVerticalMidPoint() const override;

        static  EActorType      GetActorTypeStatic() { return EActorType::SKYBOX; }
        virtual EActorType      GetActorType() const override  { return EActorType::SKYBOX; }
        virtual IActor*         CreateActorAsset(const FDString& InName) const override;
        virtual void            LoadAsset() override;

        static  CSkybox*        CreateActor(CSkybox* BaseActorResource, CWorld* InWorld, const FSkyboxDescription& InSkyboxDescription);

        /** Creates an empty actor asset that lazily loads it's resources when referenced for the first time */
        static  CSkybox*        CreateEmptyActorAsset(const FDString& InName);

#if DEVELOPMENT
        /** Editor stuff */
        virtual void UIDrawActorDetails() override;

    protected:
        virtual void InternalSaveToResourceFile(const FString& InFilePath) override;
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