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

    enum class MaterialType : u8
    {
        OPAQUE,
        TRANSLUCENT
    };

    class Material
    {
      public:
        explicit Material(gpu::Shader* CustomShader = nullptr) : customShader(CustomShader) {}

        inline MaterialType GetMaterialType() const { return type; }

        // Function responsible for sending materila's properties to the shader as uniform variables
        // It's called by the Renderer, the renderer is free to decide which shader will be actually used
        // it can use the Material's 'CustomShader' or provide some other shader as it sees fit 
        virtual void SetupShader(gpu::Shader* Shader) = 0;

        inline void SetCustomShader(gpu::Shader* Shader) { customShader = Shader; }
        inline gpu::Shader* GetCustomShader() const { return customShader; }

        virtual ~Material() = default;

      protected:

        gpu::Shader* customShader;
        MaterialType type;
    };
} // namespace lucid::scene
