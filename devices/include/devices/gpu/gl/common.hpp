#pragma once

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

    inline GLenum ToGLDataType(const Type& InType)
    {
        GLenum glType = GL_TYPES[static_cast<u8>(InType)];
        assert(glType);
        return glType;
    }

    Type ToLucidDataType(GLenum InGLType);
} // namespace lucid
