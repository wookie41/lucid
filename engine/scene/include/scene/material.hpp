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
        BLINN_PHONG_MAPS,
        TERRAIN,
        PBR,
        TEXTURED_PBR
    };

    class CMaterial
    {
      public:
        CMaterial(const UUID InID, const FDString& InName, const FDString& InResourcePath, gpu::CShader* InShader = nullptr)
        : AssetId(InID), Name(InName), AssetPath(InResourcePath), Shader(InShader)
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
         * This function advances the pointers by the amount of data it has written to the buffers and return the amount of written bytes so we know
         * how much data to send to the GPU.
         */
        virtual void SetupShaderBuffer(char* InMaterialDataPtr) { bMaterialDataDirty = false; }
        virtual void SetupPrepassShader(FForwardPrepassUniforms* InPrepassUniforms) = 0;

        inline const UUID&    GetID() const { return AssetId; };
        inline const FString& GetName() const { return Name; };
        virtual EMaterialType GetType() const { return EMaterialType::NONE; }
        inline gpu::CShader*  GetShader() const { return Shader; }
        virtual void          SaveToResourceFile(const lucid::EFileFormat& InFileFormat);
        virtual CMaterial*    GetCopy() const = 0;
        inline bool           IsMaterialDataDirty() const { return bMaterialDataDirty; }

        /** Methods used to manage textures and other resources referenced by the material */
        virtual void LoadResources() { bMaterialDataDirty = true; };
        virtual void UnloadResources(){ bMaterialDataDirty = true; };

        void CreateMaterialAsset();

        /** Calculates the size in bytes needed to store properties of this material */
        virtual u16 GetShaderDataSize() const = 0;

        virtual ~CMaterial() = default;

#if DEVELOPMENT
        virtual void UIDrawMaterialEditor();
#endif

        UUID          AssetId;
        FDString      Name;
        FDString      AssetPath;
        gpu::CShader* Shader;
        bool          bIsAsset            = false;
        i32           MaterialBufferIndex = -1;

        /**
         * Used when a mesh changes it's material.
         * These fields are set when a mesh changes it's material type so the renderer can recycle this entry
         */
        i32           MaterialBufferIndexToFree = -1;
        EMaterialType TypeToFree                = EMaterialType::NONE;

      protected:
        virtual void InternalSaveToResourceFile(const lucid::EFileFormat& InFileFormat) = 0;

        bool bMaterialDataDirty = true;
        bool bIsRenaming        = false;
    };
} // namespace lucid::scene
