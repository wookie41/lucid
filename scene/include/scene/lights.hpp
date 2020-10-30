#pragma once

#include "glm/glm.hpp"

namespace lucid::gpu
{
    class Shader;
}

namespace lucid::scene
{
    enum class LightType : uint8_t
    {
        DIRECTIONAL,
        POINT,
        SPOT
    };

    class Light
    {
      public:
        virtual LightType GetType() = 0;

        virtual ~Light() = default;
    };

    class DirectionalLight : public Light
    {
      public:
        virtual LightType GetType() { return type; }

        glm::vec3 Direction;
        glm::vec3 Color;

      private:
        static const LightType type = LightType::DIRECTIONAL;
    };

    struct PointLight : public Light
    {
      public:
        virtual LightType GetType() { return type; }

        glm::vec3 Position;
        float Constant;
        float Linear;
        float Quadratic;
        glm::vec3 Color;

      private:
        static const LightType type = LightType::POINT;
    };

} // namespace lucid::scene
