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

    inline GLenum toGLDataType(const Type& type)
    {
        GLenum glType = GL_TYPES[type];
        assert(glType);
        return glType;
    }
} // namespace lucid
