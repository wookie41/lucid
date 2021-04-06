#include "scene/world.hpp"
#include "scene/render_scene.hpp"

#include "scene/actors/static_mesh.hpp"
#include "scene/actors/skybox.hpp"
#include "scene/actors/lights.hpp"

#include "stb_ds.h"
#include "common/log.hpp"

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

    void CWorld::AddLight(CLight* InLight)
    {
        if (AddActor(InLight))
        {
            arrput(Lights, InLight);
        }
    }

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
        // If actor doesn't have an id yet or it's not in the map
        if (InActor->Id == 0 || hmgeti(ActorById, InActor->Id) == -1)
        {
            InActor->Id = NextActorId++;
            hmput(ActorById, InActor->Id, InActor);
            LUCID_LOG(ELogLevel::INFO, "Actor '%s' added to the world", *InActor->Name);
            return InActor->Id;
        }
        return 0;
    }
} // namespace lucid::scene
