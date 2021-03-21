#pragma once

#include "common/strings.hpp"

namespace lucid::gpu
{
    class CShader;
    class CTexture;
} // namespace lucid::gpu

namespace lucid::scene
{

    enum class EMaterialType : u8
    {
        OPAQUE,
        TRANSLUCENT
    };

    class CMaterial
    {
      public:
        explicit CMaterial(gpu::CShader* CustomShader = nullptr) : customShader(CustomShader) {}

        inline EMaterialType GetMaterialType() const { return type; }

        // Function responsible for sending materila's properties to the shader as uniform variables
        // It's called by the Renderer, the renderer is free to decide which shader will be actually used
        // it can use the Material's 'CustomShader' or provide some other shader as it sees fit 
        virtual void SetupShader(gpu::CShader* Shader) = 0;

        inline void SetCustomShader(gpu::CShader* Shader) { customShader = Shader; }
        inline gpu::CShader* GetCustomShader() const { return customShader; }

        virtual ~CMaterial() = default;

      protected:

        gpu::CShader* customShader;
        EMaterialType type;
    };
} // namespace lucid::scene
