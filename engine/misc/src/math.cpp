#include "misc/math.hpp"
#include <random>

namespace lucid::math
{
    float RandomFloat()
    {
        static std::default_random_engine            generator;
        static std::uniform_real_distribution<float> randomFloats(0.0, 1.0);
        return randomFloats(generator);
    }

    glm::vec2 RandomVec2() { return { RandomFloat(), RandomFloat() }; }

    glm::vec3 RandomVec3() { return { RandomFloat(), RandomFloat(), RandomFloat() }; }

    void FAABB::OrientAround(const scene::FTransform3D& Transform)
    {
        const FAABB ScaledAABB = (*this) * Transform.Scale;
        
        FrontUpperLeftCorner  = Transform.Translation + Transform.Rotation * glm::vec3{ ScaledAABB.MinX, ScaledAABB.MaxY, ScaledAABB.MaxZ };
        FrontLowerLeftCorner  = Transform.Translation + Transform.Rotation * glm::vec3{ ScaledAABB.MinX, ScaledAABB.MinY, ScaledAABB.MaxZ };
        FrontUpperRightCorner = Transform.Translation + Transform.Rotation * glm::vec3{ ScaledAABB.MaxX, ScaledAABB.MaxY, ScaledAABB.MaxZ };
        FrontLowerRightCorner = Transform.Translation + Transform.Rotation * glm::vec3{ ScaledAABB.MaxX, ScaledAABB.MinY, ScaledAABB.MaxZ };

        BackUpperLeftCorner  = Transform.Translation + Transform.Rotation * glm::vec3{ ScaledAABB.MinX, ScaledAABB.MaxY, ScaledAABB.MinZ };
        BackLowerLeftCorner  = Transform.Translation + Transform.Rotation * glm::vec3{ ScaledAABB.MinX, ScaledAABB.MinY, ScaledAABB.MinZ };
        BackUpperRightCorner = Transform.Translation + Transform.Rotation * glm::vec3{ ScaledAABB.MaxX, ScaledAABB.MaxY, ScaledAABB.MinZ };
        BackLowerRightCorner = Transform.Translation + Transform.Rotation * glm::vec3{ ScaledAABB.MaxX, ScaledAABB.MinY, ScaledAABB.MinZ };
    }

    FAABB FAABB::operator*(const glm::vec3& InScale) const
    {
        FAABB NewAABB = *this;
        NewAABB.MinX *= InScale.x;
        NewAABB.MaxX *= InScale.x;
        NewAABB.MinY *= InScale.y;
        NewAABB.MaxY *= InScale.y;
        NewAABB.MinZ *= InScale.z;
        NewAABB.MaxZ *= InScale.z;
        return NewAABB;
    }

    void FAABB::operator*=(const glm::vec3& InScale)
    {
        MinX *= InScale.x;
        MaxX *= InScale.x;
        MinY *= InScale.y;
        MaxY *= InScale.y;
        MinZ *= InScale.z;
        MaxZ *= InScale.z;
    }

    real Lerp(const real& X, const real& Y, const real& T);
} // namespace lucid::math
