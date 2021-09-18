#pragma once

#include "common/collections.hpp"
#include "common/strings.hpp"
#include "misc/math.hpp"

namespace lucid::resources
{
    class CTextureResource;
}

namespace lucid::scene
{
    class CLight;
    class CDirectionalLight;
    class CSpotLight;
    class CPointLight;
    class CStaticMesh;
    class CSkybox;
    class CTerrain;

    /*
     * The RenderScene contains things like objects to render, lights, fog volumes in a Renderer-implementation-agnostic format.
     * It's a result of running culling techniques which produce a minimal set of objects that the renderer needs to render the
     * scene.
     * The specific Renderers then use the provided scene information to render the scene in their own specific way
     */
    struct FRenderScene
    {
        FRenderScene() = default;

        FHashMap<u32, CStaticMesh*>       StaticMeshes;
        FHashMap<u32, CTerrain*>          Terrains;
        FHashMap<u32, CDirectionalLight*> DirectionalLights;
        FHashMap<u32, CSpotLight*>        SpotLights;
        FHashMap<u32, CPointLight*>       PointLights;
        FHashMap<u32, CLight*>            AllLights;
        CSkybox*                          Skybox;
    };

    struct FGeometryIntersectionQueryResult
    {
        FArray<CStaticMesh*> StaticMeshes{ 32, true };
        FArray<CTerrain*>    Terrains{ 32, true };
    };

    bool TestOverlap(const math::FAABB& A, const math::FAABB& B);

    /** Sweep A along SweepDirection to see if it can intersect B */
    bool SweptTestOverlap(const math::FAABB& A, const math::FAABB& B, const glm::vec3& SweepDirection);

    void FindGeometryOverlappingSweptAABB(const FRenderScene*               Scene,
                                          const math::FAABB&                AABB,
                                          const glm::vec3&                  SweepDirection,
                                          FGeometryIntersectionQueryResult& OutQueryResult);
} // namespace lucid::scene
