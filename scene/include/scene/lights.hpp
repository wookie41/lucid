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

        glm::vec3 Direction = { 0, 0, 0 };
        glm::vec3 Color = { 0, 0, 0 };

      private:
        static const LightType type = LightType::DIRECTIONAL;
    };

    struct PointLight : public Light
    {
      public:
        virtual LightType GetType() { return type; }

        glm::vec3 Position = { 0, 0, 0 };
        float Constant = 0;
        float Linear = 0;
        float Quadratic = 0;
        glm::vec3 Color = { 0, 0, 0 };

      private:
        static const LightType type = LightType::POINT;
    };

    struct SpotLight : public PointLight
    {
      public:
        virtual LightType GetType() { return type; }

        glm::vec3 Direction = { 0, 0, 0 };
        float InnerCutOffRad = 0;
        float OuterCutOffRad = 0;

      private:
        static const LightType type = LightType::SPOT;
    };

} // namespace lucid::scene
