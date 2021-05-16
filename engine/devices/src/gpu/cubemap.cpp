#include "devices/gpu/cubemap.hpp"

namespace lucid::gpu
{
    CCubemap::CCubemap(const FString& InName,
                       const u32& InWidth,
                       const u32& InHeight,
                       const ETextureDataType& InTextureDataType,
                       const ETextureDataFormat& InTextureDataFormat,
                       const ETexturePixelFormat& InTexturePixelFormat) : CTexture(InName, InWidth, InHeight, InTextureDataType, InTextureDataFormat, InTexturePixelFormat)
    {
    }
    
    void CCubemap::ImGuiDrawToImage(const ImVec2& InImageSize) const
    {
        // noop
    }

     bool CCubemap::ImGuiImageButton(const ImVec2& InImageSize) const
    {
        // noop
        return false;
    }

}