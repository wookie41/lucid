#include "scene/world.hpp"
#include "scene/render_scene.hpp"

#include "scene/actors/static_mesh.hpp"
#include "scene/actors/skybox.hpp"

#include "stb_ds.h"

namespace lucid::scene
{
    FRenderScene StaticRenderScene;

    /** Id for the skybox renderable, as there is always only one skybox */
    static const u32 SkyboxRenderableId = 4294967295;
    
    void CWorld::Init() {}

    void CWorld::AddStaticMesh(CStaticMesh* InStaticMesh)
    {
        if (AddActor(InStaticMesh))
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

    IActor* CWorld::GetActorById(const u32& InActorId)
    {
        if (hmgeti(ActorById, InActorId) != -1)
        {
            return hmget(ActorById, InActorId);
        }
        return nullptr;
    }

    u32 CWorld::AddActor(IActor* InActor)
    {
        if (InActor->Id == 0 || hmgeti(ActorById, InActor->Id) == -1)
        {
            InActor->Id = NextActorId++;
            hmput(ActorById, InActor->Id, InActor);
            return InActor->Id;
        }
        return 0;
    }
} // namespace lucid::scene
