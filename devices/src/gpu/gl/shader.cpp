#include "devices/gpu/gl/shader.hpp"
#include "common/collections.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/texture.hpp"
#include "devices/gpu/gpu.hpp"
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
#endif

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
            //@TODO exclude gl_ prefixed uniforms
            //https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetActiveUniform.xhtml
            glGetActiveUniform(shaderProgramID, uniformIdx, MAX_UNIFORM_VARIABLE_NAME_LENGTH, &uniformNameLength, &uniformSize,
                               &uniformType, uniformNameBuff);

            if (uniformType == GL_SAMPLER_1D || uniformType == GL_SAMPLER_2D || uniformType == GL_SAMPLER_3D)
            {
                textureBindings.Add({ uniformIdx, CopyToString(uniformNameBuff), texturesCount++ });
                continue;
            }

            uniformVariables.Add({ uniformIdx, CopyToString(uniformNameBuff) });
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

    void GLShader::Use()
    {
        glUseProgram(glShaderID);
        gpu::Info.CurrentShader = this;
    }

    void GLShader::Disable()
    {
        if (gpu::Info.CurrentShader == this)
        {
            glUseProgram(0);
            gpu::Info.CurrentShader = nullptr;
        }
    }

    void GLShader::SetupBuffersBindings()
    {
        assert(gpu::Info.CurrentShader == this);

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

    int32_t GLShader::GetIdForUniform(const String& Name) const
    {
        for (uint32_t idx = 0; idx < uniformVariables.Length; ++idx)
        {
            if (uniformVariables[idx]->Name.Hash == Name.Hash)
                return idx;
        }

#ifndef NDEBUG
        printf("Uniform variable with name %s not found in shader %d\n", (char const*)Name, glShaderID);
#endif
        return -1;
    };

    void GLShader::SetInt(const int32_t& UniformId, const uint32_t& Value)
    {
        assert(gpu::Info.CurrentShader == this);

        if (UniformId > -1 && UniformId < uniformVariables.Length)
        {
            glUniform1i(uniformVariables[UniformId]->GLIndex, Value);
        }
    }

    void GLShader::SetFloat(const int32_t& UniformId, const float& Value)
    {
        assert(gpu::Info.CurrentShader == this);

        if (UniformId > -1 && UniformId < uniformVariables.Length)
        {
            glUniform1f(uniformVariables[UniformId]->GLIndex, Value);
        }
    }

    void GLShader::SetBool(const int32_t& UniformId, const bool& Value)
    {
        assert(gpu::Info.CurrentShader == this);

        if (UniformId > -1 && UniformId < uniformVariables.Length)
        {
            glUniform1i(uniformVariables[UniformId]->GLIndex, Value);
        }
    }

    void GLShader::SetVector(const int32_t& UniformId, const glm::vec2& Value)
    {
        assert(gpu::Info.CurrentShader == this);

        if (UniformId > -1 && UniformId < uniformVariables.Length)
        {
            glUniform2fv(uniformVariables[UniformId]->GLIndex, 1, &Value[0]);
        }
    }

    void GLShader::SetVector(const int32_t& UniformId, const glm::vec3& Value)
    {
        assert(gpu::Info.CurrentShader == this);

        if (UniformId > -1 && UniformId < uniformVariables.Length)
        {
            glUniform3fv(uniformVariables[UniformId]->GLIndex, 1, &Value[0]);
        }
    }

    void GLShader::SetVector(const int32_t& UniformId, const glm::vec4& Value)
    {
        assert(gpu::Info.CurrentShader == this);

        if (UniformId > -1 && UniformId < uniformVariables.Length)
        {
            glUniform4fv(uniformVariables[UniformId]->GLIndex, 1, &Value[0]);
        }
    }

    void GLShader::SetVector(const int32_t& UniformId, const glm::ivec2& Value)
    {
        assert(gpu::Info.CurrentShader == this);

        if (UniformId > -1 && UniformId < uniformVariables.Length)
        {
            glUniform2iv(uniformVariables[UniformId]->GLIndex, 1, &Value[0]);
        }
    }

    void GLShader::SetVector(const int32_t& UniformId, const glm::ivec3& Value)
    {
        assert(gpu::Info.CurrentShader == this);

        if (UniformId > -1 && UniformId < uniformVariables.Length)
        {
            glUniform3iv(uniformVariables[UniformId]->GLIndex, 1, &Value[0]);
        }
    }

    void GLShader::SetVector(const int32_t& UniformId, const glm::ivec4& Value)
    {
        assert(gpu::Info.CurrentShader == this);

        if (UniformId > -1 && UniformId < uniformVariables.Length)
        {
            glUniform4iv(uniformVariables[UniformId]->GLIndex, 1, &Value[0]);
        }
    }

    void GLShader::SetMatrix(const int32_t& UniformId, const glm::mat4& Value)
    {
        assert(gpu::Info.CurrentShader == this);

        if (UniformId > -1 && UniformId < uniformVariables.Length)
        {
            glUniformMatrix4fv(uniformVariables[UniformId]->GLIndex, 1, GL_FALSE, &Value[0][0]);
        }
    }

    int32_t GLShader::GetTextureId(const String& TextureName)
    {
        assert(gpu::Info.CurrentShader == this);

        for (uint32_t idx = 0; idx < textureBindings.Length; ++idx)
        {
            if (textureBindings[idx]->Name.Hash == TextureName.Hash)
            {
                return idx;
            }
        }

#ifndef NDEBUG
        printf("Sampler with name %s not found in shader %d\n", (char const*)TextureName, glShaderID);
#endif
        return -1;
    }

    void GLShader::UseTexture(const int32_t& TextureId, Texture* TextureToUse)
    {
        assert(gpu::Info.CurrentShader == this);

        if (TextureId < 0 || TextureId >= textureBindings.Length)
        {
            return;
        }

        TextureBinding* binding = textureBindings[TextureId];
        binding->BoundTexture = TextureToUse;

        gpu::Info.ActiveTextureUnit = binding->TextureIndex;
        glActiveTexture(GL_TEXTURE0 + binding->TextureIndex);
        TextureToUse->Bind();
        glUniform1i(binding->GLIndex, binding->TextureIndex);
    }

    void GLShader::AddBinding(BufferBinding* Binding) { buffersBindings.Add(Binding); }

    GLShader::~GLShader() { glDeleteShader(glShaderID); }
} // namespace lucid::gpu