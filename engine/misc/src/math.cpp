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

        MinXWS = std::min({ FrontUpperLeftCorner.x,
                            FrontLowerLeftCorner.x,
                            FrontUpperRightCorner.x,
                            FrontLowerRightCorner.x,
                            BackUpperLeftCorner.x,
                            BackLowerLeftCorner.x,
                            BackUpperRightCorner.x,
                            BackLowerRightCorner.x });
        MaxXWS = std::max({ FrontUpperLeftCorner.x,
                            FrontLowerLeftCorner.x,
                            FrontUpperRightCorner.x,
                            FrontLowerRightCorner.x,
                            BackUpperLeftCorner.x,
                            BackLowerLeftCorner.x,
                            BackUpperRightCorner.x,
                            BackLowerRightCorner.x });

        MinYWS = std::min({ FrontUpperLeftCorner.y,
                            FrontLowerLeftCorner.y,
                            FrontUpperRightCorner.y,
                            FrontLowerRightCorner.y,
                            BackUpperLeftCorner.y,
                            BackLowerLeftCorner.y,
                            BackUpperRightCorner.y,
                            BackLowerRightCorner.y });

        MaxYWS = std::max({ FrontUpperLeftCorner.y,
                            FrontLowerLeftCorner.y,
                            FrontUpperRightCorner.y,
                            FrontLowerRightCorner.y,
                            BackUpperLeftCorner.y,
                            BackLowerLeftCorner.y,
                            BackUpperRightCorner.y,
                            BackLowerRightCorner.y });

        MinZWS = std::min({ FrontUpperLeftCorner.z,
                            FrontLowerLeftCorner.z,
                            FrontUpperRightCorner.z,
                            FrontLowerRightCorner.z,
                            BackUpperLeftCorner.z,
                            BackLowerLeftCorner.z,
                            BackUpperRightCorner.z,
                            BackLowerRightCorner.z });
        MaxZWS = std::max({ FrontUpperLeftCorner.z,
                            FrontLowerLeftCorner.z,
                            FrontUpperRightCorner.z,
                            FrontLowerRightCorner.z,
                            BackUpperLeftCorner.z,
                            BackLowerLeftCorner.z,
                            BackUpperRightCorner.z,
                            BackLowerRightCorner.z });
    }

    float FAABB::GetMinWS(const u8& Axis) const
    {
        if (Axis == 0)
        {
            return MinXWS;
        }
        if (Axis == 1)
        {
            return MinYWS;
        }
        if (Axis == 2)
        {
            return MinZWS;
        }
        assert(0);
        return 0;
    }

    float FAABB::GetMaxWS(const u8& Axis) const
    {
        if (Axis == 0)
        {
            return MaxXWS;
        }
        if (Axis == 1)
        {
            return MaxYWS;
        }
        if (Axis == 2)
        {
            return MaxZWS;
        }
        assert(0);
        return 0;
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

    glm::vec3 FAABB::GetFrustumCenter() const
    {
        const glm::vec3 V            = BackUpperRightCorner - FrontLowerLeftCorner;
        const float     HalfDistance = glm::length(V) / 2;
        return FrontLowerLeftCorner + (glm::normalize(V) * HalfDistance);
    }

    glm::vec3 FAABB::operator[](const int& InCorner)
    {
        if (InCorner == 0)
        {
            return FrontUpperLeftCorner;
        }
        if (InCorner == 1)
        {
            return FrontLowerLeftCorner;
        }
        if (InCorner == 2)
        {
            return FrontUpperRightCorner;
        }
        if (InCorner == 3)
        {
            return FrontLowerRightCorner;
        }
        if (InCorner == 4)
        {
            return BackUpperLeftCorner;
        }
        if (InCorner == 5)
        {
            return BackLowerLeftCorner;
        }
        if (InCorner == 6)
        {
            return BackUpperRightCorner;
        }
        if (InCorner == 7)
        {
            return BackLowerRightCorner;
        }

        assert(0);
        return glm::vec3{ 0 };
    }

    void FAABB::TranslateCorner(const int& InCorner, const glm::vec3& InTranslate)
    {
        if (InCorner == 0)
        {
            FrontUpperLeftCorner += InTranslate;
            return;
        }

        if (InCorner == 1)
        {
            FrontLowerLeftCorner += InTranslate;
            return;
        }

        if (InCorner == 2)
        {
            FrontUpperRightCorner += InTranslate;
            return;
        }

        if (InCorner == 3)
        {
            FrontLowerRightCorner += InTranslate;
            return;
        }

        if (InCorner == 4)
        {
            BackUpperLeftCorner += InTranslate;
            return;
        }

        if (InCorner == 5)
        {
            BackLowerLeftCorner += InTranslate;
            return;
        }

        if (InCorner == 6)
        {
            BackUpperRightCorner += InTranslate;
            return;
        }

        if (InCorner == 7)
        {
            BackLowerRightCorner += InTranslate;
            return;
        }

        assert(0);
    }

    void FAABB::GrowInWorldSpace(const math::FAABB& Other)
    {
        MinXWS = std::min(MinXWS, Other.MinXWS);
        MaxXWS = std::max(MaxXWS, Other.MaxXWS);
        MinYWS = std::min(MinYWS, Other.MinYWS);
        MaxYWS = std::max(MaxYWS, Other.MaxYWS);
        MinZWS = std::min(MinZWS, Other.MinZWS);
        MaxZWS = std::max(MaxZWS, Other.MaxZWS);
    }

    real Lerp(const real& X, const real& Y, const real& T);
} // namespace lucid::math
