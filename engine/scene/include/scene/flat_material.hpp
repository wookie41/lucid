#pragma once

#include "scene/material.hpp"
#include "common/types.hpp"
#include "schemas/types.hpp"

namespace lucid::gpu
{
    class CShader;
};

namespace lucid::scene
{
    class CFlatMaterial : public CMaterial
    {
      public:
        static CFlatMaterial* CreateMaterial(const FFlatMaterialDescription& Description, const FDString& InResourcePath);

        CFlatMaterial(const UUID& InId, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader);

        virtual void SetupShader(gpu::CShader* Shader) override;

        void SaveToResourceFile(const lucid::EFileFormat& InFileFormat) const override;
        
        FColor Color;
    };

}