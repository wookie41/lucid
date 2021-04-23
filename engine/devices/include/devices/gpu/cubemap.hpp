#pragma once

#include "devices/gpu/texture.hpp"

namespace lucid::resources
{
    class CTextureResource;
}

namespace lucid::gpu
{
    class CCubemap : public CTexture
    {
      public:
        CCubemap(const FString& InName,
                 const u32& InWidth,
                 const u32& InHeight,
                 const ETextureDataType InTextureDataType,
                 const ETexturePixelFormat InTexturePixelFormat);

        enum class EFace : u8
        {
            RIGHT,
            LEFT,
            UP,
            DOWN,
            BACK,
            FRONT
        };

        virtual void AttachAsColor(const u8& Index, EFace InFace) = 0;

#if DEVELOPMENT
        virtual void ImGuiDrawToImage(const ImVec2& InImageSize) const;
        virtual bool ImGuiImageButton(const ImVec2& InImageSize) const;
#endif
        virtual ~CCubemap() = default;
    };

    CCubemap* CreateCubemap(const u32& Width,
                            const u32& Height,
                            ETextureDataFormat InDataFormat,
                            ETexturePixelFormat InPixelFormat,
                            ETextureDataType DataType,
                            const void* FaceTexturesData[6],
                            const FString& InName);

} // namespace lucid::gpu