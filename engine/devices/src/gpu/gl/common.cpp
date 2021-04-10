#include "devices/gpu/gl/common.hpp"

namespace lucid
{
    EType ToLucidDataType(GLenum InGLType)
    {
        switch (InGLType)
        {
        case GL_INT:
            return EType::INT_32;
        case GL_FLOAT:
            return EType::FLOAT;
        case GL_BOOL:
            return EType::BOOL;
        case GL_FLOAT_VEC2:
            return EType::VEC2;
        case GL_FLOAT_VEC3:
            return EType::VEC3;
        case GL_FLOAT_VEC4:
            return EType::VEC4;
        case GL_INT_VEC2:
            return EType::IVEC2;
        case GL_INT_VEC3:
            return EType::IVEC3;
        case GL_INT_VEC4:
            return EType::IVEC4;
        case GL_FLOAT_MAT4:
            return EType::MAT4;
        }
        return EType::UNSUPPORTED;
    }

    void SetGLObjectName(GLenum InIdentifier, GLuint InGLName, const FString& InName)
    {
        glObjectLabel(InIdentifier, InGLName, InName.GetLength(), *InName);
    }
}
