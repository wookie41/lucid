#pragma once

#include <glm/ext/scalar_int_sized.hpp>

#include "common/types.hpp"
#include "platform/input.hpp"

namespace lucid::scene
{
    class   CStaticMesh;
    class   CSkybox;
    class   CLight;
    class   CCamera;
    class   IRenderable;
    struct  FRenderScene;
    
    /**
     * World represents all of the currently loaded renderable objects and is culled based on camera's position
     * to produce a CRenderScene which is then handed out the Renderer that actually renders the scene.
     * There can be objects dynamically to and unloaded from the world.
     */
    class CWorld
    {

    public:

        void            Init();
        void            AddStaticMesh(CStaticMesh* InStaticMesh);
        void            AddLight(CLight* InLight);
        void            SetSkybox(CSkybox* InSkybox);
        
        FRenderScene*   MakeRenderScene(CCamera* InCamera);
        IRenderable*    GetRenderableById(const u32& RenderableId);

    private:
        u32             AddRenderable(IRenderable* InRenderable);
        
        struct
        {
            u32             key; // Renderable id
            IRenderable*    value; //
        }* RenderableById = NULL;

        u32             NextRenderableId = 1;
        
        CStaticMesh**   StaticMeshes = nullptr;
        CLight**        Lights = nullptr;
        CSkybox*        Skybox = nullptr;
    };
}
