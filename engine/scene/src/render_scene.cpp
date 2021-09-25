#include "scene/render_scene.hpp"

#include "scene/actors/static_mesh.hpp"
#include "scene/actors/terrain.hpp"

namespace lucid::scene
{
    bool TestOverlap(const math::FAABB& A, const math::FAABB& B)
    {
        return (A.MinXWS <= B.MaxXWS && A.MaxXWS >= B.MinXWS) && (A.MinYWS <= B.MaxYWS && A.MaxYWS >= B.MinYWS) &&
               (A.MinZWS <= B.MaxZWS && A.MaxZWS >= B.MinZWS);
    }

    bool SweptTestOverlap(const math::FAABB& A, const math::FAABB& B, const glm::vec3& SweepDirection)
    {
        if (TestOverlap(A, B))
        {
            return true;
        }

        // first times of overlap along each axis
        glm::vec3 uv0(0, 0, 0);

        // last times of overlap along each axis
        glm::vec3 uv1(FLT_MAX, FLT_MAX, FLT_MAX);

        // the problem is solved in A's frame of reference
        const glm::vec3 V = -SweepDirection;

        // find the possible first and last times of overlap along each axis
        for (u8 i = 0; i < 3; i++)
        {
            if (A.GetMaxWS(i) < B.GetMinWS(i) && V[i] < 0)
            {
                uv0[i] = (A.GetMaxWS(i) - B.GetMinWS(i)) / V[i];
            }
            else if (B.GetMaxWS(i) < A.GetMinWS(i) && V[i] > 0)
            {
                uv0[i] = (A.GetMinWS(i) - B.GetMaxWS(i)) / V[i];
            }
            if (B.GetMaxWS(i) > A.GetMinWS(i) && V[i] < 0)
            {
                uv1[i] = (A.GetMinWS(i) - B.GetMaxWS(i)) / V[i];
            }
            else if (A.GetMaxWS(i) > B.GetMinWS(i) && V[i] > 0)
            {
                uv1[i] = (A.GetMaxWS(i) - B.GetMinWS(i)) / V[i];
            }
        }

        // possible first time of overlap
        float u0 = glm::max(uv0.x, glm::max(uv0.y, uv0.z));

        // possible last time of overlap
        float u1 = glm::min(uv1.x, glm::min(uv1.y, uv1.z));

        // they could have only collided if the first time of
        // overlap occurred before the last time of overlap
        if (u0 > 0 && u0 <= u1)
        {
            const glm::vec3 Displacement = SweepDirection * u0;
            math::FAABB     SweptA       = A;

            SweptA.MinXWS += Displacement.x;
            SweptA.MaxXWS += Displacement.x;

            SweptA.MinYWS += Displacement.y;
            SweptA.MaxYWS += Displacement.y;

            SweptA.MinZWS += Displacement.z;
            SweptA.MaxZWS += Displacement.z;

            return TestOverlap(SweptA, B);
        }

        return false;
    }

    void FindGeometryOverlappingSweptAABB(const FRenderScene*               Scene,
                                          const math::FAABB&                AABB,
                                          const glm::vec3&                  SweepDirection,
                                          FGeometryIntersectionQueryResult& OutQueryResult)
    {
        OutQueryResult.GeometryAABB.MinX = FLT_MAX;
        OutQueryResult.GeometryAABB.MinY = FLT_MAX;
        OutQueryResult.GeometryAABB.MinZ = FLT_MAX;
        OutQueryResult.GeometryAABB.MaxX = 0;
        OutQueryResult.GeometryAABB.MaxY = 0;
        OutQueryResult.GeometryAABB.MaxZ = 0;

        for (int i = 0; i < Scene->StaticMeshes.GetLength(); ++i)
        {
            CStaticMesh* StaticMesh = Scene->StaticMeshes.GetByIndex(i);
            if (SweptTestOverlap(AABB, StaticMesh->GetAABB(), SweepDirection))
            {
                OutQueryResult.StaticMeshes.Add(StaticMesh);
            }
        }

        for (int i = 0; i < Scene->Terrains.GetLength(); ++i)
        {
            CTerrain* Terrain = Scene->Terrains.GetByIndex(i);
            if (SweptTestOverlap(Terrain->GetAABB(), AABB, SweepDirection))
            {
                OutQueryResult.Terrains.Add(Terrain);
            }
        }
    }
}; // namespace lucid::scene
