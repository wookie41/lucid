#pragma once

#include "common/strings.hpp"
#include "GL/glew.h"
#include "common/types.hpp"

namespace lucid
{
    static const GLenum GL_TYPES[] = { GL_BYTE,
                                       GL_BYTE,
                                       GL_SHORT,
                                       GL_INT,
                                       0,

                                       GL_BOOL,

                                       GL_UNSIGNED_BYTE,
                                       GL_UNSIGNED_SHORT,
                                       GL_UNSIGNED_INT,
                                       0,

                                       GL_FLOAT,
                                       GL_DOUBLE,

                                       0,
                                       0,
                                       0,

                                       0,
                                       0,
                                       0,

                                       0,
                                       0 };

    inline GLenum ToGLDataType(const EType& InType)
    {
        GLenum glType = GL_TYPES[static_cast<u8>(InType)];
        assert(glType);
        return glType;
    }

    EType ToLucidDataType(GLenum InGLType);

    void SetGLObjectName(GLenum InIdentifier, GLuint InGLName, const FString& InName);


    static GLenum GL_MIN_FILTERS_MAPPING[] = { GL_NEAREST,
                                               GL_LINEAR,
                                               GL_NEAREST_MIPMAP_NEAREST,
                                               GL_LINEAR_MIPMAP_NEAREST,
                                               GL_NEAREST_MIPMAP_LINEAR,
                                               GL_LINEAR_MIPMAP_LINEAR };

    static GLenum GL_MAG_FILTERS_MAPPING[] = { GL_NEAREST, GL_LINEAR };
    static GLenum GL_WRAP_FILTERS_MAPPING[] = { GL_CLAMP_TO_EDGE, GL_MIRRORED_REPEAT, GL_REPEAT, GL_CLAMP_TO_BORDER };
    static GLenum GL_TEXTURE_DATA_TYPE_MAPPING[] = { GL_UNSIGNED_BYTE, GL_FLOAT, GL_UNSIGNED_INT };
    static GLenum GL_TEXTURE_TARGET_MAPPING[] = { GL_TEXTURE_1D, GL_TEXTURE_2D, GL_TEXTURE_3D };
    static GLenum GL_TEXTURE_DATA_FORMAT[] = {
        GL_RED,    GL_R16F, GL_R32F,    GL_R32UI,   GL_RG,   GL_RG16F,      GL_RG32F,           GL_RGB,          GL_RGB16F,
        GL_RGB32F, GL_RGBA, GL_RGBA16F, GL_RGBA32F, GL_SRGB, GL_SRGB_ALPHA, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL
    };
    static GLenum GL_TEXTURE_PIXEL_FORMAT[] = { GL_RED,  GL_RED_INTEGER,     GL_RG,           GL_RGB,
                                                GL_RGBA, GL_DEPTH_COMPONENT, GL_DEPTH_STENCIL };

#define TO_GL_MIN_FILTER(gl_filters) (GL_MIN_FILTERS_MAPPING[(u8)gl_filters])
#define TO_GL_MAG_FILTER(gl_filters) (GL_MAG_FILTERS_MAPPING[(u8)gl_filters])
#define TO_GL_WRAP_FILTER(gl_filters) (GL_WRAP_FILTERS_MAPPING[(u8)gl_filters])
#define TO_GL_TEXTURE_DATA_TYPE(type) (GL_TEXTURE_DATA_TYPE_MAPPING[static_cast<u8>(type)])
#define TO_GL_TEXTURE_TARGET(type) (GL_TEXTURE_TARGET_MAPPING[static_cast<u8>(type)])
#define TO_GL_TEXTURE_DATA_FORMAT(type) (GL_TEXTURE_DATA_FORMAT[static_cast<u8>(type)])
#define TO_GL_TEXTURE_PIXEL_FORMAT(type) (GL_TEXTURE_PIXEL_FORMAT[static_cast<u8>(type)])

    
} // namespace lucid
