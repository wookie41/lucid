#include "devices/gpu/gl/shader.hpp"
#include "common/collections.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/texture.hpp"
#include "devices/gpu/gpu.hpp"
#include "common/log.hpp"

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

    Shader::Shader(const String& InName) : Name(InName) {}

    Shader* CompileShaderProgram(const String& ShaderName,
                                 const String& VertexShaderSource,
                                 const String& FragementShaderSource,
                                 const bool& WarnMissingUniforms)
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
            // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetActiveUniform.xhtml
            glGetActiveUniform(shaderProgramID, uniformIdx, MAX_UNIFORM_VARIABLE_NAME_LENGTH, &uniformNameLength, &uniformSize,
                               &uniformType, uniformNameBuff);

            if (uniformType == GL_SAMPLER_1D || uniformType == GL_SAMPLER_2D || uniformType == GL_SAMPLER_3D ||
                uniformType == GL_SAMPLER_CUBE)
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

        return new GLShader(ShaderName, shaderProgramID, uniformVariables, textureBindings, WarnMissingUniforms);
    }

    GLShader::GLShader(const String& InName,
                       const GLuint& GLShaderID,
                       StaticArray<UniformVariable> UniformVariables,
                       StaticArray<TextureBinding> TextureBindings,
                       const bool& WarnMissingUniforms = false)
    : Shader::Shader(InName), glShaderID(GLShaderID), uniformVariables(UniformVariables), textureBindings(TextureBindings),
      warnMissingUniforms(WarnMissingUniforms)

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

    void GLShader::SetInt(const String& Name, const uint32_t& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        auto uniformId = GetIdForUniform(Name);

        if (uniformId > -1 && uniformId < uniformVariables.Length)
        {
            glUniform1i(uniformVariables[uniformId]->GLIndex, Value);
        }
    }

    void GLShader::SetFloat(const String& Name, const float& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        auto uniformId = GetIdForUniform(Name);

        if (uniformId > -1 && uniformId < uniformVariables.Length)
        {
            glUniform1f(uniformVariables[uniformId]->GLIndex, Value);
        }
    }

    void GLShader::SetBool(const String& Name, const bool& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        auto uniformId = GetIdForUniform(Name);

        if (uniformId > -1 && uniformId < uniformVariables.Length)
        {
            glUniform1i(uniformVariables[uniformId]->GLIndex, Value);
        }
    }

    void GLShader::SetVector(const String& Name, const glm::vec2& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        auto uniformId = GetIdForUniform(Name);

        if (uniformId > -1 && uniformId < uniformVariables.Length)
        {
            glUniform2fv(uniformVariables[uniformId]->GLIndex, 1, &Value[0]);
        }
    }

    void GLShader::SetVector(const String& Name, const glm::vec3& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        auto uniformId = GetIdForUniform(Name);

        if (uniformId > -1 && uniformId < uniformVariables.Length)
        {
            glUniform3fv(uniformVariables[uniformId]->GLIndex, 1, &Value[0]);
        }
    }

    void GLShader::SetVector(const String& Name, const glm::vec4& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        auto uniformId = GetIdForUniform(Name);

        if (uniformId > -1 && uniformId < uniformVariables.Length)
        {
            glUniform4fv(uniformVariables[uniformId]->GLIndex, 1, &Value[0]);
        }
    }

    void GLShader::SetVector(const String& Name, const glm::ivec2& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        auto uniformId = GetIdForUniform(Name);

        if (uniformId > -1 && uniformId < uniformVariables.Length)
        {
            glUniform2iv(uniformVariables[uniformId]->GLIndex, 1, &Value[0]);
        }
    }

    void GLShader::SetVector(const String& Name, const glm::ivec3& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        auto uniformId = GetIdForUniform(Name);

        if (uniformId > -1 && uniformId < uniformVariables.Length)
        {
            glUniform3iv(uniformVariables[uniformId]->GLIndex, 1, &Value[0]);
        }
    }

    void GLShader::SetVector(const String& Name, const glm::ivec4& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        auto uniformId = GetIdForUniform(Name);

        if (uniformId > -1 && uniformId < uniformVariables.Length)
        {
            glUniform4iv(uniformVariables[uniformId]->GLIndex, 1, &Value[0]);
        }
    }

    void GLShader::SetMatrix(const String& Name, const glm::mat4& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        auto uniformId = GetIdForUniform(Name);

        if (uniformId > -1 && uniformId < uniformVariables.Length)
        {
            glUniformMatrix4fv(uniformVariables[uniformId]->GLIndex, 1, GL_FALSE, &Value[0][0]);
        }
    }

    void GLShader::UseTexture(const String& Name, Texture* TextureToUse)
    {
        assert(gpu::Info.CurrentShader == this);
        auto uniformId = GetTextureId(Name);

        if (uniformId < 0 || uniformId >= textureBindings.Length)
        {
            return;
        }

        TextureBinding* binding = textureBindings[uniformId];
        binding->BoundTexture = TextureToUse;

        gpu::Info.ActiveTextureUnit = binding->TextureIndex;
        glActiveTexture(GL_TEXTURE0 + binding->TextureIndex);
        TextureToUse->Bind();
        glUniform1i(binding->GLIndex, binding->TextureIndex);
    }

    void GLShader::AddBinding(BufferBinding* Binding) { buffersBindings.Add(Binding); }

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
        if (warnMissingUniforms)
        {
            LUCID_LOG(LogLevel::WARN, "Uniform variable with name %s not found in shader %s\n", (char const*)Name, (char const*)this->Name);
        }
#endif
        return -1;
    };

    int32_t GLShader::GetTextureId(const String& Name) const
    {
        assert(gpu::Info.CurrentShader == this);

        for (uint32_t idx = 0; idx < textureBindings.Length; ++idx)
        {
            if (textureBindings[idx]->Name.Hash == Name.Hash)
            {
                return idx;
            }
        }

#ifndef NDEBUG
        if (warnMissingUniforms)
        {
            LUCID_LOG(LogLevel::WARN, "Sampler with name %s not found in shader %s\n", (char const*)Name, (char const*)this->Name);
        }
#endif
        return -1;
    }

    GLShader::~GLShader() { glDeleteShader(glShaderID); }
} // namespace lucid::gpu