#pragma once

#include "common/strings.hpp"

namespace lucid::gpu
{
    class CShader;
    class CTexture;
} // namespace lucid::gpu

namespace lucid
{
    enum class EFileFormat : int;
}
namespace lucid::scene
{
    enum class EMaterialType : u8
    {
        NONE,
        FLAT,
        BLINN_PHONG,
        BLINN_PHONG_MAPS
    };
    
    class CMaterial
    {
      public:
        CMaterial(const UUID InID, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader = nullptr)
        : ID(InID), Name(InName), ResourcePath(InResourcePath), Shader(InShader)
        {
        }

        /*
         * Function responsible for sending material's properties to the shader as uniform variables
         * It's called by the Renderer, the renderer is free to decide which shader will be actually used
         * it can use the Material's 'Shader' or provide some other shader as it sees fit - e.x. the renderer
         * will use a shadow map shader to "render" geometry when generating shadow maps
         */
        virtual void            SetupShader(gpu::CShader* InShadder) = 0;
        inline const UUID&      GetID() const { return ID; };
        inline const FString&   GetName() const { return Name; };
        virtual EMaterialType   GetType() const { return EMaterialType::NONE; }
        inline gpu::CShader*    GetShader() const { return Shader; }
        virtual void            SaveToResourceFile(const lucid::EFileFormat& InFileFormat);
        virtual CMaterial*      GetCopy() const = 0;

        void CreateMaterialAsset();

        virtual ~CMaterial() = default;

#if DEVELOPMENT
        virtual void UIDrawMaterialEditor() = 0;
#endif
    
        UUID            ID;
        FDString        Name;
        FDString        ResourcePath;
        gpu::CShader*   Shader;
        bool            bIsAsset = false;

    protected:
        virtual void            InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat) = 0;

    };
} // namespace lucid::scene
