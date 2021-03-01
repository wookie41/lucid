#include "devices/gpu/gl/common.hpp"

namespace lucid
{
    Type ToLucidDataType(GLenum GLType)
    {
        switch (GLType)
        {
        case GL_INT:
            return Type::INT_32;
        case GL_FLOAT:
            return Type::FLOAT;
        case GL_BOOL:
            return Type::BOOL;
        case GL_FLOAT_VEC2:
            return Type::VEC2;
        case GL_FLOAT_VEC3:
            return Type::VEC3;
        case GL_FLOAT_VEC4:
            return Type::VEC4;
        case GL_INT_VEC2:
            return Type::IVEC2;
        case GL_INT_VEC3:
            return Type::IVEC3;
        case GL_INT_VEC4:
            return Type::IVEC4;
        case GL_FLOAT_MAT4:
            return Type::MAT4;
        }
        return Type::UNSUPPORTED;
    }
}
