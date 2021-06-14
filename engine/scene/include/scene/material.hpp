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
    struct FForwardPrepassUniforms;

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
         * Function responsible for sending material's properties to the shader uniform buffers
         * The uniform buffers get mem-mapped by the renderer and pointers to the place where the material should be
         * written in passed in this function's arguments.
         * If the material uses textures, then it should write bindless handles to InBindlessTextureArrayPtr
         * It's called by the Renderer, the renderer is free to decide which shader will be actually used
         * it can use the Material's 'Shader' or provide some other shader as it sees fit - e.x. the renderer
         * will use a shadow map shader to "render" geometry when generating shadow maps
         * This function advances the pointers by the amount of data it has written to the buffers and writes the amount of written bytes so we know
         * how much data to send to the GPU.
         */
        virtual void          SetupShaderBuffers(char* InMaterialDataPtr, u64* InBindlessTexturesArrayPtr, u32& OutMaterialDataSize, u32& BindlessTexturesSize) = 0;
        virtual void          SetupPrepassShaderBuffers(FForwardPrepassUniforms* InPrepassUniforms) = 0;
        
        inline const UUID&    GetID() const { return ID; };
        inline const FString& GetName() const { return Name; };
        virtual EMaterialType GetType() const { return EMaterialType::NONE; }
        inline gpu::CShader*  GetShader() const { return Shader; }
        virtual void          SaveToResourceFile(const lucid::EFileFormat& InFileFormat);
        virtual CMaterial*    GetCopy() const = 0;

        /** Methods used to manage textures and other resources referenced by the material */
        virtual void LoadResources(){};
        virtual void UnloadResources(){};

        void CreateMaterialAsset();

        /** Calculates the size in bytes needed to store properties of this material */
        virtual u16 GetShaderDataSize() const = 0;

        virtual ~CMaterial() = default;

#if DEVELOPMENT
        virtual void UIDrawMaterialEditor();
#endif

        UUID          ID;
        FDString      Name;
        FDString      ResourcePath;
        gpu::CShader* Shader;
        bool          bIsAsset = false;

      protected:
        virtual void InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat) = 0;

        bool bIsRenaming = false;
    };
} // namespace lucid::scene
