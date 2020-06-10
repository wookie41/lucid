#include "devices/gpu/gl/shader.hpp"
#include "common/collections.hpp"
#include "devices/gpu/buffer.hpp"

#ifndef NDEBUG
#include <stdio.h>
#endif

namespace lucid::gpu
{
    const uint8_t _GL_PROGRAM = 0;
    const uint8_t _GL_VERTEX_SHADER = 1;
    const uint8_t _GL_FRAGMENT_SHADER = 2;

    const uint8_t MAX_UNIFORM_VARIABLE_NAME_LENGTH = 255;

#ifndef NDEBUG
    static char _infoLog[1024];
#endif

    void checkCompileErrors(unsigned int Shader, const uint8_t& Type)
    {
        int success;
        if (Type != _GL_PROGRAM)
        {
            glGetShaderiv(Shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(Shader, 1024, NULL, _infoLog);
                printf("[OpenGL] Failed to compile shader: %s\n", _infoLog);
            }
        }
        else
        {
            glGetProgramiv(Shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(Shader, 1024, NULL, _infoLog);
                printf("[OpenGL] Failed to link shader program: %s\n", _infoLog);
            }
        }
    }

    void compileShader(GLuint shaderProgramID, GLuint shaderID, const char* shaderSource, const uint8_t& Type)
    {
        glShaderSource(shaderID, 1, &shaderSource, NULL);
        glCompileShader(shaderID);
#ifndef NDEBUG
        checkCompileErrors(shaderID, Type);
#endif
        glAttachShader(shaderProgramID, shaderID);
    }

    Shader* CompileShaderProgram(const String& VertexShaderSource, const String& FragementShaderSource)
    {
        GLuint shaderProgramID = glCreateProgram();
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        compileShader(shaderProgramID, vertexShader, VertexShaderSource, _GL_VERTEX_SHADER);
        compileShader(shaderProgramID, fragmentShader, FragementShaderSource, _GL_FRAGMENT_SHADER);

        glLinkProgram(shaderProgramID);

#ifndef NDEBUG
        checkCompileErrors(shaderProgramID, _GL_PROGRAM);
#endif

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        GLint numberOfUniforms;
        glGetProgramiv(shaderProgramID, GL_ACTIVE_UNIFORMS, &numberOfUniforms);

        StaticArray<UniformVariable> uniformVariables(numberOfUniforms);
        StaticArray<UniformVariable> samplerUniforms(numberOfUniforms);

        GLint uniformSize;
        GLenum uniformType;
        GLsizei uniformNameLength;

        char uniformNameBuff[MAX_UNIFORM_VARIABLE_NAME_LENGTH];

        for (GLint uniformIdx = 0; uniformIdx < numberOfUniforms; ++uniformIdx)
        {
            glGetActiveUniform(shaderProgramID, uniformIdx, MAX_UNIFORM_VARIABLE_NAME_LENGTH,
                               &uniformNameLength, &uniformSize, &uniformType, uniformNameBuff);

            bool isSampler = uniformType == GL_SAMPLER_2D; // TODO add other kinds of sampler types when needed
            if (isSampler)
            {
                samplerUniforms.Add({ uniformIdx, CopyString(uniformNameBuff) });
                continue;
            }

            uniformVariables.Add({ uniformIdx, CopyString(uniformNameBuff) });
        }

        if (samplerUniforms.Length > 0)
        {
            StaticArray<UniformVariable> tmpUniforms = uniformVariables.Copy();
            uniformVariables.Free();
            uniformVariables = tmpUniforms;
        }

        return new GLShader(shaderProgramID, uniformVariables, samplerUniforms);
    }

    GLShader::GLShader(const GLuint& GLShaderID,
                       StaticArray<UniformVariable> UniformVariables,
                       StaticArray<UniformVariable> SamplerUniforms)
    : glShaderID(GLShaderID), uniformVariables(UniformVariables), samplerUniforms(SamplerUniforms)

    {
    }


    void GLShader::Use() const { glUseProgram(glShaderID); }

    void GLShader::Disable() const { glUseProgram(0); }

    void GLShader::SetInt(const String& UniformName, const uint32_t& Value) const
    {
        glUniform1i(getUniformLocation(UniformName, false), Value);
    }

    void GLShader::SetFloat(const String& UniformName, const float& Value) const
    {
        glUniform1f(getUniformLocation(UniformName, false), Value);
    }

    void GLShader::SetBool(const String& UniformName, const bool& Value) const
    {
        glUniform1i(getUniformLocation(UniformName, false), Value);
    }

    void GLShader::SetVector(const String& UniformName, const math::vec2& Value) const
    {
        glUniform2fv(getUniformLocation(UniformName, false), 1, (float*)&Value);
    }

    void GLShader::SetVector(const String& UniformName, const math::vec3& Value) const
    {
        glUniform3fv(getUniformLocation(UniformName, false), 1, (float*)&Value);
    }

    void GLShader::SetVector(const String& UniformName, const math::vec4& Value) const
    {
        glUniform4fv(getUniformLocation(UniformName, false), 1, (float*)&Value);
    }

    void GLShader::SetVector(const String& UniformName, const math::ivec2& Value) const
    {
        glUniform2iv(getUniformLocation(UniformName, false), 1, (GLint*)&Value);
    }

    void GLShader::SetVector(const String& UniformName, const math::ivec3& Value) const
    {
        glUniform3iv(getUniformLocation(UniformName, false), 1, (GLint*)&Value);
    }

    void GLShader::SetVector(const String& UniformName, const math::ivec4& Value) const
    {
        glUniform4iv(getUniformLocation(UniformName, false), 1, (GLint*)&Value);
    }

    GLint GLShader::getUniformLocation(const String& Name, const bool& IsSampler) const
    {
        const StaticArray<UniformVariable>& uniformsArray = IsSampler ? samplerUniforms : uniformVariables;

        for (uint32_t idx = 0; idx < uniformsArray.Length; ++idx)
        {
            if (uniformsArray[idx]->Name.Hash == Name.Hash)
                return idx;
        }

#ifndef NDEBUG
        printf("Uniform variable with name %s not found in shader %d", Name.CString, glShaderID);
#endif
        return -1;
    }
} // namespace lucid::gpu