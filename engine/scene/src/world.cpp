#include "scene/world.hpp"

#include <atlalloc.h>

#include "scene/render_scene.hpp"

#include "stb_ds.h"

namespace lucid::scene
{
    FRenderScene StaticRenderScene;

    /** Id for the skybox renderable, as there is always only one skybox */
    static const u32 SkyboxRenderableId = 4294967295;
    
    void CWorld::Init() {}

    void CWorld::AddStaticMesh(CStaticMesh* InStaticMesh)
    {
        if (AddRenderable(InStaticMesh))
        {
            arrput(StaticMeshes, InStaticMesh);            
        }
    }

    void CWorld::AddLight(CLight* InLight) { arrput(Lights, InLight); }

    void CWorld::SetSkybox(CSkybox* InSkybox)
    {
        if (Skybox)
        {
            Skybox->Id = 0;
        }
        
        Skybox = InSkybox;
        Skybox->Id = SkyboxRenderableId;
    }

    FRenderScene* CWorld::MakeRenderScene(CCamera* InCamera)
    {
        StaticRenderScene.Lights = Lights;
        StaticRenderScene.StaticMeshes = StaticMeshes;
        StaticRenderScene.Skybox = Skybox;
        return &StaticRenderScene;
    }

    IRenderable* CWorld::GetRenderableById(const u32& RenderableId)
    {
        if (hmgeti(RenderableById, RenderableId) != -1)
        {
            return hmget(RenderableById, RenderableId);
        }
        return nullptr;
    }

    u32 CWorld::AddRenderable(IRenderable* InRenderable)
    {
        if (InRenderable->Id == 0 || hmgeti(RenderableById, InRenderable->Id) == -1)
        {
            InRenderable->Id = NextRenderableId++;
            hmput(RenderableById, InRenderable->Id, InRenderable);
            return InRenderable->Id;
        }
        return 0;
    }
} // namespace lucid::scene
