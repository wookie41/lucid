#pragma once

#include "common/collections.hpp"
#include "common/strings.hpp"
#include "glm/glm.hpp"

namespace lucid::gpu
{
    class Shader;
    class Texture;
} // namespace lucid::gpu

namespace lucid::scene
{

    enum class MaterialType : uint8_t
    {
        OPAQUE,
        TRANSLUCENT
    };

    class Material
    {
      public:
        explicit Material(gpu::Shader* CustomShader = nullptr) : customShader(CustomShader) {}

        inline MaterialType GetMaterialType() const { return type; }

        virtual void SetupShader(gpu::Shader* Shader) = 0;

        inline void SetCustomShader(gpu::Shader* Shader) { customShader = Shader; }
        inline gpu::Shader* GetCustomShader() const { return customShader; }

        virtual ~Material() = default;

      protected:
        gpu::Shader* customShader;
        MaterialType type;
    };
} // namespace lucid::scene
