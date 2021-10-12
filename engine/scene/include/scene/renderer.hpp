#pragma once

#include "enums.hpp"
#include "scene/actors/lights.hpp"
#include "settings.hpp"
#include "devices/gpu/texture.hpp"
#include "devices/gpu/viewport.hpp"
#include "platform/input.hpp"
#include "scene/render_scene.hpp"

namespace lucid::gpu
{
    class CFramebuffer;
    class CShader;
    class CTexture;
}; // namespace lucid::gpu

namespace lucid::scene
{
    class CCamera;

    struct FRenderView
    {
        CCamera*       Camera;
        gpu::FViewport Viewport;
    };

    struct FDebugLine
    {
        FDebugLine(const glm::vec3&  InStart,
                   const glm::vec3&  InEnd,
                   const glm::vec3&  InStartColor,
                   const glm::vec3&  InEndColor,
                   const float&      InTime,
                   const ESpaceType& InSpaceType)
        : Start(InStart), End(InEnd), StartColor(InStartColor), EndColor(InEndColor), RemoveTime(InTime), SpaceType(InSpaceType)
        {
        }

        glm::vec3  Start{ 0 };
        glm::vec3  StartColor{ 0 };
        glm::vec3  End{ 0 };
        glm::vec3  EndColor{ 0 };
        float      RemoveTime{ 0 };
        ESpaceType SpaceType = WORLD_SPACE;
    };

    /////////////////////////////////////
    //         RendererObjects         //
    /////////////////////////////////////

    using RID = u32;

    class CRendererObject
    {
      public:
        explicit CRendererObject(const RID& InId) : Id(InId) {}

        inline RID GetId() const { return Id; }

        virtual void Free() = 0;

        virtual ~CRendererObject() = default;

      protected:
        RID Id;
    };

    /////////////////////////////////////
    //           ShadowMaps            //
    /////////////////////////////////////
    class CShadowMap : public CRendererObject
    {
      public:
        CShadowMap(const RID& InId, gpu::CTexture* InShadowMapTexture, const u8& InShadowMapQuality);

        inline gpu::CTexture* GetShadowMapTexture() const { return ShadowMapTexture; }
        inline gpu::CCubemap* GetShadowCubeMapTexture() const { return ShadowCubeMapTexture; }
        inline u8             GetQuality() const { return ShadowMapQuality; }

        virtual void Free() override;

      private:
        u8             ShadowMapQuality;
        gpu::CTexture* ShadowMapTexture     = nullptr;
        gpu::CCubemap* ShadowCubeMapTexture = nullptr;
    };

#if DEVELOPMENT

    template <typename T>
    struct FCachedTexture
    {
        inline T GetValueAtPosition(const glm::vec2& MousePosition) const
        {
            if (MousePosition.x > 0 && (int)MousePosition.x < Width && MousePosition.y > 0 && (int)MousePosition.y < Height)
            {
                return CachedTextureData[(Width * (Height - (int)MousePosition.y)) + (int)MousePosition.x];
            }
            return 0;
        }

        T*  CachedTextureData;
        u32 Width, Height;
    };

    /** Stores information about the last rendered frame, populated by the renderer in Render() */
    struct FRenderStats
    {
        float FrameTimeMiliseconds;
        u32   NumDrawCalls;
        u64   FrameNumber = 0;
    };

    extern FRenderStats GRenderStats;

#endif

    /////////////////////////////////////
    //            Renderer             //
    /////////////////////////////////////
    class CRenderer
    {
      public:
        /////////////////////////////////////
        //        Renderer interface       //
        /////////////////////////////////////

        /**
         * Called before the first Render() call so that the renderer can set itself up, like create additional framebuffers and
         * etc. Can be called multiple time, subsequent Setup() calls have be preceded by Cleanup() calls
         */
        virtual void Setup() = 0;

        /**
         * Renders the scene from the specified view
         */
        virtual void Render(FRenderScene* InSceneToRender, const FRenderView* InRenderView) = 0;

        /**
         * Called before the renderer is deleted so it can cleanup whatever it did in Setup() or during Render(s)().
         */
        virtual void Cleanup() = 0;

        virtual void ResetState() = 0;

        /**
         * Returns the framebuffer which holds the result of last Render() call.
         */
        virtual gpu::CFramebuffer* GetResultFramebuffer()  = 0;
        virtual gpu::CTexture*     GetResultFrameTexture() = 0;

        /** Stalls the CPU until GPU finished executing all commands of the previous Render() call.  */
        void WaitForFrameEnd() const { gpu::Finish(); }

        /////////////////////////////////////
        //         Lights/ShadowMaps       //
        /////////////////////////////////////

        CDirectionalLight* CreateDirectionalLight(const FDString& InName, IActor* InParent, CWorld* InWorld, const bool& CastsShadow, const u8& InCascadeCount);
        CSpotLight*        CreateSpotLight(const FDString& InName, IActor* InParent, CWorld* InWorld, const bool& CastsShadow);
        CPointLight*       CreatePointLight(const FDString& InName, IActor* InParent, CWorld* InWorld, const bool& CastsShadow);

        CShadowMap* CreateShadowMap(const ELightType& InLightType);
        void        RemoveShadowMap(CShadowMap* InShadowMap);

#if DEVELOPMENT
        inline const FCachedTexture<u32>&   GetCachedHitMap() const { return CachedHitMap; }
        inline const FCachedTexture<float>& GetCachedDistanceToCameraMap() const { return CachedDistanceToCameraMap; }
        inline gpu::CTexture*               GetLightBulbTexture() const { return LightBulbTexture; }
        inline gpu::CTexture*               GetSelectedDebugTexture() const { return SelectedDebugTexture; }
        
        /**
         * Queues a debug line to draw during the next Render() call.
         * The debug line will persist for n seconds after it's added.
         * If you want to  draw a line only for one frame, then pass 0 as InPersistTime.
         */
        void DrawDebugLine(const glm::vec3&  InStart,
                           const glm::vec3&  InEnd,
                           const glm::vec3&  InStartColor,
                           const glm::vec3&  InEndColor,
                           const float&      InPersistTime = 0,
                           const ESpaceType& InSpaceType   = WORLD_SPACE);

        void DrawAABB(const math::FAABB& InAABB, const FColor& InColor);

#endif

        virtual ~CRenderer() = default;

        glm::uvec2 ResultResolution;

      protected:
#if DEVELOPMENT
        void RemoveStaleDebugLines();

        const u16               MaxDebugLines = 1024;
        std::vector<FDebugLine> DebugLines;

      public:
        virtual bool UIDrawSettingsWindow() = 0;

#endif

        /** Lights and shadow maps arrays */
        CLight**     CreatedLights     = nullptr;
        CShadowMap** CreatedShadowMaps = nullptr;

        /** Default quality used when creating  shadow maps - 0 lowest */
        u8 DefaultShadowMapQuality = 1;
        u8 DefaultLightQuality     = 1;

        /** Size of a shadow map for a given quality  */
        const glm::uvec2 ShadowMapSizeByQuality[3] = {
            { 512, 512 },
            { 1024, 1024 },
            { 2048, 2048 },
        };

        const LightSettings LightSettingsByQuality[3] = { { -10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 10.f },
                                                          { -10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 35.f },
                                                          { -10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 70.f } };

#if DEVELOPMENT
      protected:
        /** Used to visualize light sources in the editor */
        gpu::CTexture* LightBulbTexture = nullptr;

        FCachedTexture<u32>   CachedHitMap              = {};
        FCachedTexture<float> CachedDistanceToCameraMap = {};

        gpu::CTexture* SelectedDebugTexture = nullptr;
#endif
    };

    /** Collection of lines needed to draw a debug arrow with arrow heads */
    struct FDebugArrow
    {
        glm::vec3 BodyStart;
        glm::vec3 BodyEnd;

        glm::vec3 HeadStart0;
        glm::vec3 HeadEnd0;

        glm::vec3 HeadStart1;
        glm::vec3 HeadEnd1;
    };

    FDebugArrow MakeDebugArrowData(const glm::vec3& InStart, const glm::vec3& InDirection, const float& InLength);

    void DrawDebugSphere(const glm::vec3& InCenter, const float& InRadius, const glm::vec3& InColor);
} // namespace lucid::scene
