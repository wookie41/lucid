#pragma once

#include <common/strings.hpp>

#include "common/types.hpp"
#include "platform/input.hpp"
#include "schemas/types.hpp"

namespace lucid::scene
{
    class   CStaticMesh;
    class   CSkybox;
    class   CLight;
    class   CDirectionalLight;
    class   CSpotLight;
    class   CPointLight;
    class   CCamera;
    class   IActor;
    struct  FRenderScene;
    
    /**
     * World represents all of the currently loaded actor and is culled based on camera's position
     * to produce a CRenderScene which is then handed out the Renderer that actually renders the scene.
     * There can be objects dynamically to and unloaded from the world.
     */
    class CWorld
    {

    public:

        void            Init();
        void            AddStaticMesh(CStaticMesh* InStaticMesh);
        void            AddDirectionalLight(CDirectionalLight* InLight);
        void            AddSpotLight(CSpotLight* InLight);
        void            AddPointLight(CPointLight* InLight);
        void            SetSkybox(CSkybox* InSkybox);
        
        FRenderScene*   MakeRenderScene(CCamera* InCamera);
        IActor*         GetActorById(const u32& InActorId);

        void            SaveToJSONFile(const FDString& InFilePath) const;
        void            SaveToBinaryFile(const FDString& InFilePath) const;
    private:

        void            CreateWorldDescription(FWorldDescription& InWorldDescription) const;
        u32             AddActor(IActor* InActor);
        
        struct
        {
            u32             key; // Actor id
            IActor*    value; 
        }* ActorById = NULL;

        u32             NextActorId = 1;
        
        CStaticMesh**       StaticMeshes = nullptr;
        CDirectionalLight** DirectionalLights = nullptr;
        CSpotLight**        SpotLights = nullptr;
        CPointLight**       PointLights = nullptr;
        CLight**            AllLights = nullptr;
        CSkybox*            Skybox = nullptr;
    };

    CWorld* LoadWorldFromJSONFile(const FString& InFilePath);
    CWorld* LoadWorldFromBinaryFile(const FString& InFilePath);
    CWorld* LoadWorld(const FWorldDescription& InWorldDescription);
}
