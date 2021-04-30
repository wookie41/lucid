#pragma once

#include "common/strings.hpp"

namespace lucid::gpu
{
    class CShader;
    class CTexture;
} // namespace lucid::gpu

namespace lucid::scene
{
    class CMaterial
    {
      public:
        explicit CMaterial(const FString& InName, gpu::CShader* InShader = nullptr) : Name(InName), Shader(InShader) {}

        /*
         * Function responsible for sending material's properties to the shader as uniform variables
         * It's called by the Renderer, the renderer is free to decide which shader will be actually used
         * it can use the Material's 'Shader' or provide some other shader as it sees fit - e.x. the renderer
         * will use a shadow map shader to "render" geometry when generating shadow maps
         */
        virtual void            SetupShader(gpu::CShader* InShadder) = 0;
        inline gpu::CShader*    GetShader() const { return Shader; }
        inline const FString&   GetName() const { return Name; };

        virtual ~CMaterial() = default;

      protected:

        const FString& Name;
        gpu::CShader* Shader;
    };
} // namespace lucid::scene
