#include "scene/world.hpp"
#include "scene/render_scene.hpp"

#include "stb_ds.h"

namespace lucid::scene
{
    FRenderScene StaticRenderScene;

    void CWorld::Init()
    {
    }

    void CWorld::AddStaticMesh(CStaticMesh* InStaticMesh) { arrput(StaticMeshes, InStaticMesh); }

    void CWorld::AddLight(CLight* InLight) { arrput(Lights, InLight); }

    void CWorld::SetSkybox(CSkybox* InSkybox) { Skybox = InSkybox; }

    FRenderScene* CWorld::MakeRenderScene(CCamera* InCamera)
    {
        StaticRenderScene.Lights = Lights;
        StaticRenderScene.StaticMeshes = StaticMeshes;
        StaticRenderScene.Skybox = Skybox;
        return &StaticRenderScene;
    }

} // namespace lucid::scene
