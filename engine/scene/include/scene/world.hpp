#pragma once

#include "common/strings.hpp"

#include "common/types.hpp"
#include "platform/input.hpp"
#include "schemas/types.hpp"

namespace lucid::scene
{
    class CStaticMesh;
    class CSkybox;
    class CLight;
    class CDirectionalLight;
    class CSpotLight;
    class CPointLight;
    class CCamera;
    class IActor;
    struct FRenderScene;

    /**
     * World represents all of the currently loaded actor and is culled based on camera's position
     * to produce a CRenderScene which is then handed out the Renderer that actually renders the scene.
     * There can be objects dynamically to and unloaded from the world.
     */
    class CWorld
    {

      public:
        void Init();

        void AddStaticMesh(CStaticMesh* InStaticMesh);
        void RemoveStaticMesh(const u32& InId);

        void AddDirectionalLight(CDirectionalLight* InLight);
        void RemoveDirectionalLight(const u32& InId);

        void AddSpotLight(CSpotLight* InLight);
        void RemoveSpotLight(const u32& InId);

        void AddPointLight(CPointLight* InLight);
        void RemovePointLight(const u32& InId);

        void            SetSkybox(CSkybox* InSkybox);
        inline CSkybox* GetSkybox() const { return Skybox; }

        IActor* RemoveActorById(const u32& InActorId, const bool& InbHardRemove);

        FRenderScene* MakeRenderScene(CCamera* InCamera);
        IActor*       GetActorById(const u32& InActorId);

        void SaveToJSONFile(const FString& InFilePath) const;
        void SaveToBinaryFile(const FString& InFilePath) const;

        inline FHashMap<u32, IActor*>& GetActorsMap() { return ActorById; }

        void Unload();

      private:
        void CreateWorldDescription(FWorldDescription& OutWorldDescription) const;
        u32  AddActor(IActor* InActor);

        FHashMap<u32, IActor*> ActorById;

        u32                               NextActorId = 1;
        FHashMap<u32, CStaticMesh*>       StaticMeshes;
        FHashMap<u32, CDirectionalLight*> DirectionalLights;
        FHashMap<u32, CSpotLight*>        SpotLights;
        FHashMap<u32, CPointLight*>       PointLights;
        FHashMap<u32, CLight*>            AllLights;
        CSkybox*                          Skybox = nullptr;
    };

    CWorld* LoadWorldFromJSONFile(const FString& InFilePath);
    CWorld* LoadWorldFromBinaryFile(const FString& InFilePath);
    CWorld* LoadWorld(const FWorldDescription& InWorldDescription);
} // namespace lucid::scene
