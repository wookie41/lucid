#pragma once

#include "devices/gpu/texture.hpp"

namespace lucid::gpu
{
    class CCubemap : public CTexture
    {
      public:
        explicit CCubemap(const FANSIString& InName,
                          const ETextureDataType InTextureDataType,
                          const ETexturePixelFormat InTexturePixelFormat)
        : CTexture(InName, InTextureDataType, InTexturePixelFormat)
        {
        }

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

        virtual ~CCubemap() = default;
    };

    CCubemap* CreateCubemap(const glm::ivec2& Size,
                            ETextureDataFormat InDataFormat,
                            ETexturePixelFormat InPixelFormat,
                            ETextureDataType DataType,
                            const char* FacesData[6],
                            const FANSIString& InName);

} // namespace lucid::gpu