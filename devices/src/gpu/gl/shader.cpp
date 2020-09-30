#include "devices/gpu/gl/shader.hpp"
#include "common/collections.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/texture.hpp"

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
        StaticArray<TextureBinding> textureBindings(numberOfUniforms);

        GLint uniformSize;
        GLenum uniformType;
        GLsizei uniformNameLength;

        uint8_t texturesCount = 0;

        char uniformNameBuff[MAX_UNIFORM_VARIABLE_NAME_LENGTH];

        for (GLint uniformIdx = 0; uniformIdx < numberOfUniforms; ++uniformIdx)
        {
            //@TODO exclude gl_ prefixed uniforms https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetActiveUniform.xhtml
            glGetActiveUniform(shaderProgramID, uniformIdx, MAX_UNIFORM_VARIABLE_NAME_LENGTH,
                               &uniformNameLength, &uniformSize, &uniformType, uniformNameBuff);

            if (uniformType == GL_SAMPLER_1D || uniformType == GL_SAMPLER_2D || uniformType == GL_SAMPLER_3D)
            {
                textureBindings.Add({ uniformIdx, CopyString(uniformNameBuff), texturesCount++ });
                continue;
            }

            uniformVariables.Add({ uniformIdx, CopyString(uniformNameBuff) });
        }

        StaticArray<UniformVariable> tmpUniforms = uniformVariables.Copy();
        uniformVariables.Free();
        uniformVariables = tmpUniforms;

        StaticArray<TextureBinding> tmpBindings = textureBindings.Copy();
        textureBindings.Free();
        textureBindings = tmpBindings;

        return new GLShader(shaderProgramID, uniformVariables, textureBindings);
    }

    GLShader::GLShader(const GLuint& GLShaderID,
                       StaticArray<UniformVariable> UniformVariables,
                       StaticArray<TextureBinding> TextureBindings)
    : glShaderID(GLShaderID), uniformVariables(UniformVariables), textureBindings(TextureBindings)

    {
    }

    void GLShader::Use() { glUseProgram(glShaderID); }

    void GLShader::Disable() { glUseProgram(0); }

    void GLShader::SetupTextureBindings()
    {
        for (uint32_t idx = 0; idx < textureBindings.Length; ++idx)
        {
            if (textureBindings[idx]->BoundTexture != nullptr)
            {
                glActiveTexture(GL_TEXTURE0 + textureBindings[idx]->TextureIndex);
                textureBindings[idx]->BoundTexture->Bind();
            }
        }
    }

    void GLShader::SetupBuffersBindings()
    {
        auto bindingNode = &buffersBindings.Head;
        while (bindingNode && bindingNode->Element)
        {
            auto binding = bindingNode->Element;
            if (binding->Index >= 0)
            {
                binding->BufferToUse->BindIndexed(binding->Index, binding->BindPoint);
            }
            else
            {
                binding->BufferToUse->Bind(binding->BindPoint);
            }

            bindingNode = bindingNode->Next;
        }
    }
    void GLShader::SetInt(const String& UniformName, const uint32_t& Value)
    {
        glUniform1i(getUniformLocation(UniformName), Value);
    }

    void GLShader::SetFloat(const String& UniformName, const float& Value)
    {
        glUniform1f(getUniformLocation(UniformName), Value);
    }

    void GLShader::SetBool(const String& UniformName, const bool& Value)
    {
        glUniform1i(getUniformLocation(UniformName), Value);
    }

    void GLShader::SetVector(const String& UniformName, const math::vec2& Value)
    {
        glUniform2fv(getUniformLocation(UniformName), 1, (float*)&Value);
    }

    void GLShader::SetVector(const String& UniformName, const math::vec3& Value)
    {
        glUniform3fv(getUniformLocation(UniformName), 1, (float*)&Value);
    }

    void GLShader::SetVector(const String& UniformName, const math::vec4& Value)
    {
        glUniform4fv(getUniformLocation(UniformName), 1, (float*)&Value);
    }

    void GLShader::SetVector(const String& UniformName, const math::ivec2& Value)
    {
        glUniform2iv(getUniformLocation(UniformName), 1, (GLint*)&Value);
    }

    void GLShader::SetVector(const String& UniformName, const math::ivec3& Value)
    {
        glUniform3iv(getUniformLocation(UniformName), 1, (GLint*)&Value);
    }

    void GLShader::SetVector(const String& UniformName, const math::ivec4& Value)
    {
        glUniform4iv(getUniformLocation(UniformName), 1, (GLint*)&Value);
    }

    void GLShader::SetMatrix(const String& UniformName, const math::mat4& Value)
    {
        glUniformMatrix4fv(getUniformLocation(UniformName), 1, GL_FALSE, (GLfloat*)&Value);
    }

    void GLShader::UseTexture(const String& TextureName, Texture* TextureToUse)
    {
        TextureBinding* binding = nullptr;
        for (uint32_t idx = 0; idx < textureBindings.Length; ++idx)
        {
            if (textureBindings[idx]->Name.Hash == TextureName.Hash)
            {
                binding = textureBindings[idx];
                break;
            }
        }

#ifndef NDEBUG
        if (binding == nullptr)
        {
            printf("Sampler with name %s not found in shader %d\n", TextureName.CString, glShaderID);
            return;
        }
#endif
        binding->BoundTexture = TextureToUse;

        glActiveTexture(GL_TEXTURE0 + binding->TextureIndex);
        TextureToUse->Bind();

        glUniform1i(binding->TextureIndex, binding->TextureIndex);
    }

    GLint GLShader::getUniformLocation(const String& Name) const
    {
        //@TODO cache it
        for (uint32_t idx = 0; idx < uniformVariables.Length; ++idx)
        {
            if (uniformVariables[idx]->Name.Hash == Name.Hash)
                return uniformVariables[idx]->GLIndex;
        }

#ifndef NDEBUG
        printf("Uniform variable with name %s not found in shader %d\n", Name.CString, glShaderID);
#endif
        return -1;
    }

    void GLShader::AddBinding(BufferBinding* Binding) { buffersBindings.Add(Binding); }

    GLShader::~GLShader() { glDeleteShader(glShaderID); }
} // namespace lucid::gpu