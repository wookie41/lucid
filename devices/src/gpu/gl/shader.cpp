#include "devices/gpu/gl/shader.hpp"

#include "common/collections.hpp"
#include "devices/gpu/buffer.hpp"
#include "devices/gpu/texture.hpp"
#include "devices/gpu/gpu.hpp"
#include "common/log.hpp"
#include "stb.h"
#include "devices/gpu/gl/common.hpp"

#ifndef NDEBUG
#include <stdio.h>
#endif

#include <string>
#include <algorithm>

namespace lucid::gpu
{
    const u8 _GL_PROGRAM = 0;
    const u8 _GL_VERTEX_SHADER = 1;
    const u8 _GL_FRAGMENT_SHADER = 2;
    const u8 _GL_GEOMETRY_SHADER = 3;

    const u8 MAX_UNIFORM_VARIABLE_NAME_LENGTH = 255;

    static const char VERTEX_SHADER_TYPE_NAME[] = "vertex";
    static const char GEOMETRY_SHADER_TYPE_NAME[] = "geometry";
    static const char FRAGMENT_SHADER_TYPE_NAME[] = "fragment";

#ifndef NDEBUG
    static char _infoLog[1024];
    void CheckCompileErrors(const FANSIString& ShaderName, const u32& Shader, const u8& Type, const char* ShaderTypeName)
    {
        int success;
        if (Type != _GL_PROGRAM)
        {
            glGetShaderiv(Shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                Zero(_infoLog, 1024);
                GLsizei len;
                glGetShaderInfoLog(Shader, 1024, &len, _infoLog);
                LUCID_LOG(ELogLevel::WARN, "[OpenGL] Failed to compile %s shader of shader program %s: %s", ShaderTypeName, *ShaderName, _infoLog);
            }
        }
        else
        {
            glGetProgramiv(Shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(Shader, 1024, NULL, _infoLog);
                LUCID_LOG(ELogLevel::WARN, "[OpenGL] Failed to link shader program %s: %s\n", *ShaderName, _infoLog);
            }
        }
    }
#endif

    void CompileShader(const FANSIString& ShaderName,
                       GLuint ShaderProgramID,
                       GLuint ShaderID,
                       const char* ShaderSource,
                       const u8& Type,
                       const char* ShaderTypeName)
    {
        glShaderSource(ShaderID, 1, &ShaderSource, NULL);
        glCompileShader(ShaderID);
#ifndef NDEBUG
        CheckCompileErrors(ShaderName, ShaderID, Type, ShaderTypeName);
#endif
        glAttachShader(ShaderProgramID, ShaderID);
    }

    CShader::CShader(const FANSIString& InName) : ShaderName(InName) {}

    CShader* CompileShaderProgram(
        const FANSIString& InShaderName,
        const FANSIString& InVertexShaderSource,
        const FANSIString& InFragementShaderSource,
        const FANSIString& InGeometryShaderSource,
        const bool& InWarnMissingUniforms)
    {
        GLuint ShaderProgramID = glCreateProgram();
        GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
        GLuint GeometryShader = 0;
        GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

        CompileShader(InShaderName, ShaderProgramID, VertexShader, *InVertexShaderSource, _GL_VERTEX_SHADER, VERTEX_SHADER_TYPE_NAME);

        if (InGeometryShaderSource.GetLength())
        {
            GeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
            CompileShader(InShaderName, ShaderProgramID, GeometryShader, *InGeometryShaderSource, _GL_GEOMETRY_SHADER,GEOMETRY_SHADER_TYPE_NAME);
        }

        CompileShader(InShaderName, ShaderProgramID, FragmentShader, *InFragementShaderSource, _GL_FRAGMENT_SHADER,FRAGMENT_SHADER_TYPE_NAME);

        glLinkProgram(ShaderProgramID);

#ifndef NDEBUG
        CheckCompileErrors(InShaderName, ShaderProgramID, _GL_PROGRAM, "");
#endif

        glDeleteShader(VertexShader);
        glDeleteShader(FragmentShader);

        if (GeometryShader)
        {
            glDeleteShader(GeometryShader);            
        }

        GLint numberOfUniforms;
        glGetProgramiv(ShaderProgramID, GL_ACTIVE_UNIFORMS, &numberOfUniforms);

        FArray<FUniformVariable> uniformVariables(numberOfUniforms, true);
        FArray<FTextureBinding> textureBindings(numberOfUniforms, true);

        GLint UniformArraySize;
        GLenum GLUniformType;
        GLsizei GLUniformNameLength;

        u8 texturesCount = 0;

        char UniformNameBuff[MAX_UNIFORM_VARIABLE_NAME_LENGTH];

        for (GLint UniformIdx = 0; UniformIdx < numberOfUniforms; ++UniformIdx)
        {
            //@TODO exclude gl_ prefixed uniforms
            // https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGetActiveUniform.xhtml
            glGetActiveUniform(ShaderProgramID, UniformIdx, MAX_UNIFORM_VARIABLE_NAME_LENGTH, &GLUniformNameLength,
                               &UniformArraySize, &GLUniformType, UniformNameBuff);

            GLint GLUniformLocation = glGetUniformLocation(ShaderProgramID, UniformNameBuff);
            EType UniformType = ToLucidDataType(GLUniformType);

            if (GLUniformType == GL_SAMPLER_1D || GLUniformType == GL_SAMPLER_2D || GLUniformType == GL_SAMPLER_3D ||
                GLUniformType == GL_SAMPLER_CUBE)
            {
                FTextureBinding NewTextureVariable;
                NewTextureVariable.Location = GLUniformLocation;
                NewTextureVariable.Name = CopyToString(UniformNameBuff, GLUniformNameLength);
                NewTextureVariable.Type = EType::SAMPLER;
                NewTextureVariable.TextureIndex = texturesCount++;
                NewTextureVariable.BoundTexture = nullptr;
                textureBindings.Add(NewTextureVariable);
                continue;
            }

            FUniformVariable NewVariable;
            NewVariable.Location = GLUniformLocation;
            NewVariable.Name = CopyToString(UniformNameBuff, GLUniformNameLength);
            NewVariable.Type = UniformType;

            uniformVariables.Add(NewVariable);

            if (UniformArraySize > 1)
            {
                // the uniform is an array and OpenGL return active uniform only for the 0th index of the array
                // length of the array is equal to uniformSize and thats how many additional uniforms we have to add
                for (int i = 1; i < UniformArraySize; ++i)
                {
                    std::string NextArrayEntry { *NewVariable.Name };
                    NextArrayEntry.replace(NextArrayEntry.find("0"), 1, std::to_string(i));

                    FUniformVariable ArrayEntryVariable;
                    ArrayEntryVariable.Location = glGetUniformLocation(ShaderProgramID, NextArrayEntry.c_str());
                    ArrayEntryVariable.Name = CopyToString(NextArrayEntry.c_str(), NextArrayEntry.length());
                    ArrayEntryVariable.Type = UniformType;
                    uniformVariables.Add(ArrayEntryVariable);
                }
            }
        }

        FArray<FUniformVariable> tmpUniforms = uniformVariables.Copy();
        uniformVariables.Free();
        uniformVariables = tmpUniforms;

        FArray<FTextureBinding> tmpBindings = textureBindings.Copy();
        textureBindings.Free();
        textureBindings = tmpBindings;

        return new CGLShader(InShaderName, ShaderProgramID, uniformVariables, textureBindings, InWarnMissingUniforms);
    }

    CGLShader::CGLShader(const FANSIString& InName,
                       const GLuint& GLShaderID,
                       FArray<FUniformVariable> UniformVariables,
                       FArray<FTextureBinding> TextureBindings,
                       const bool& WarnMissingUniforms = false)
    : CShader::CShader(InName), glShaderID(GLShaderID), uniformVariables(UniformVariables), textureBindings(TextureBindings),
      warnMissingUniforms(WarnMissingUniforms)

    {
    }

    void CGLShader::Use()
    {
        glUseProgram(glShaderID);
        gpu::Info.CurrentShader = this;
    }

    void CGLShader::Disable()
    {
        if (gpu::Info.CurrentShader == this)
        {
            glUseProgram(0);
            gpu::Info.CurrentShader = nullptr;
        }
    }

    void CGLShader::SetInt(const FANSIString& InUniformName, const uint32_t& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        uint32 UniformId = GetIdForUniform(InUniformName);

        if (UniformId < uniformVariables.GetLength())
        {
            glUniform1i(uniformVariables[UniformId]->Location, Value);
        }
    }

    void CGLShader::SetFloat(const FANSIString& InUniformName, const float& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        const u32 UniformId = GetIdForUniform(InUniformName);

        if (UniformId < uniformVariables.GetLength())
        {
            glUniform1f(uniformVariables[UniformId]->Location, Value);
        }
    }

    void CGLShader::SetBool(const FANSIString& InUniformName, const bool& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        const u32 UniformId = GetIdForUniform(InUniformName);

        if (UniformId < uniformVariables.GetLength())
        {
            glUniform1i(uniformVariables[UniformId]->Location, Value);
        }
    }

    void CGLShader::SetVector(const FANSIString& InUniformName, const glm::vec2& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        const u32 UniformId = GetIdForUniform(InUniformName);

        if (UniformId < uniformVariables.GetLength())
        {
            glUniform2fv(uniformVariables[UniformId]->Location, 1, &Value[0]);
        }
    }

    void CGLShader::SetVector(const FANSIString& InUniformName, const glm::vec3& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        const u32 UniformId = GetIdForUniform(InUniformName);

        if (UniformId < uniformVariables.GetLength())
        {
            glUniform3fv(uniformVariables[UniformId]->Location, 1, &Value[0]);
        }
    }

    void CGLShader::SetVector(const FANSIString& InUniformName, const glm::vec4& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        const u32 UniformId = GetIdForUniform(InUniformName);

        if (UniformId < uniformVariables.GetLength())
        {
            glUniform4fv(uniformVariables[UniformId]->Location, 1, &Value[0]);
        }
    }

    void CGLShader::SetVector(const FANSIString& InUniformName, const glm::ivec2& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        const u32 UniformId = GetIdForUniform(InUniformName);

        if (UniformId < uniformVariables.GetLength())
        {
            glUniform2iv(uniformVariables[UniformId]->Location, 1, &Value[0]);
        }
    }

    void CGLShader::SetVector(const FANSIString& InUniformName, const glm::ivec3& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        const u32 UniformId = GetIdForUniform(InUniformName);

        if (UniformId < uniformVariables.GetLength())
        {
            glUniform3iv(uniformVariables[UniformId]->Location, 1, &Value[0]);
        }
    }

    void CGLShader::SetVector(const FANSIString& InUniformName, const glm::ivec4& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        const u32 UniformId = GetIdForUniform(InUniformName);

        if (UniformId < uniformVariables.GetLength())
        {
            glUniform4iv(uniformVariables[UniformId]->Location, 1, &Value[0]);
        }
    }

    void CGLShader::SetMatrix(const FANSIString& InUniformName, const glm::mat4& Value)
    {
        assert(gpu::Info.CurrentShader == this);
        const u32 UniformId = GetIdForUniform(InUniformName);

        if (UniformId < uniformVariables.GetLength())
        {
            glUniformMatrix4fv(uniformVariables[UniformId]->Location, 1, GL_FALSE, &Value[0][0]);
        }
    }

    void CGLShader::UseTexture(const FANSIString& InUniformName, CTexture* TextureToUse)
    {
        assert(gpu::Info.CurrentShader == this);
        const i32 UniformId = GetTextureId(InUniformName);

        if (UniformId == textureBindings.GetLength())
        {
            return;
        }

        FTextureBinding* binding = textureBindings[UniformId];
        binding->BoundTexture = TextureToUse;

        gpu::Info.ActiveTextureUnit = binding->TextureIndex;
        glActiveTexture(GL_TEXTURE0 + binding->TextureIndex);
        TextureToUse->Bind();
        glUniform1i(binding->Location, binding->TextureIndex);
    }

    void CGLShader::RestoreTextureBindings()
    {
        assert(gpu::Info.CurrentShader == this);

        for (u32 i = 0; i < textureBindings.GetLength(); ++i)
        {
            if (textureBindings[i]->BoundTexture != nullptr)
            {
                gpu::Info.ActiveTextureUnit = textureBindings[i]->TextureIndex;
                glActiveTexture(GL_TEXTURE0 + textureBindings[i]->TextureIndex);
                textureBindings[i]->BoundTexture->Bind();
                glUniform1i(textureBindings[i]->Location, textureBindings[i]->TextureIndex);
            }
        }
    }

    void CGLShader::AddBinding(BufferBinding* Binding) { buffersBindings.Add(Binding); }

    void CGLShader::ReloadShader(CShader* RecompiledShader)
    {
        // Preserve the current shader
        CShader* CurrentShader = nullptr;
        const GLuint OldProgramId = glShaderID;
        if (Info.CurrentShader == this)
        {
            Disable();
            CurrentShader = this;
        }
        else if (Info.CurrentShader != nullptr)
        {
            Info.CurrentShader->Disable();
            CurrentShader = Info.CurrentShader;
        }

        // Get the new program
        CGLShader* GLRecompiledShader = (CGLShader*)RecompiledShader;
        glShaderID = GLRecompiledShader->glShaderID;

        // Use the new shader
        Use();

        // Preserve uniforms values
        for (u32 i = 0; i < GLRecompiledShader->uniformVariables.GetLength(); ++i)
        {
            // Check if the old shader has this variable too
            const u32 OldUniformIndex = GetIdForUniform(GLRecompiledShader->uniformVariables[i]->Name);
            if (OldUniformIndex == uniformVariables.GetLength())
            {
                continue;
            }

            const GLint OldUniformLocation = uniformVariables[OldUniformIndex]->Location;
            switch (uniformVariables[OldUniformIndex]->Type)
            {
            case EType::BOOL:
            case EType::INT_32:
            {
                i32 Value;
                glGetUniformiv(OldProgramId, OldUniformLocation, &Value);
                glUniform1i(GLRecompiledShader->uniformVariables[i]->Location, Value);
                break;
            }
            case EType::FLOAT:
            {
                float Value;
                glGetUniformfv(OldProgramId, OldUniformLocation, &Value);
                glUniform1f(GLRecompiledShader->uniformVariables[i]->Location, Value);
                break;
            }
            case EType::VEC2:
            {
                glm::vec2 Value;
                glGetUniformfv(OldProgramId, OldUniformLocation, &Value[0]);
                glUniform2fv(GLRecompiledShader->uniformVariables[i]->Location, 1, &Value[0]);
                break;
            }
            case EType::VEC3:
            {
                glm::vec3 Value;
                glGetUniformfv(OldProgramId, OldUniformLocation, &Value[0]);
                glUniform3fv(GLRecompiledShader->uniformVariables[i]->Location, 1, &Value[0]);
                break;
            }

            case EType::VEC4:
            {
                glm::vec4 Value;
                glGetUniformfv(OldProgramId, OldUniformLocation, &Value[0]);
                glUniform4fv(GLRecompiledShader->uniformVariables[i]->Location, 1, &Value[0]);
                break;
            }
            case EType::IVEC2:
            {
                glm::ivec2 Value;
                glGetUniformiv(OldProgramId, OldUniformLocation, &Value[0]);
                glUniform2iv(GLRecompiledShader->uniformVariables[i]->Location, 1, &Value[0]);
                break;
            }
            case EType::IVEC3:
            {
                glm::ivec3 Value;
                glGetUniformiv(OldProgramId, OldUniformLocation, &Value[0]);
                glUniform3iv(GLRecompiledShader->uniformVariables[i]->Location, 1, &Value[0]);
                break;
            }

            case EType::IVEC4:
            {
                glm::ivec4 Value;
                glGetUniformiv(OldProgramId, OldUniformLocation, &Value[0]);
                glUniform4iv(GLRecompiledShader->uniformVariables[i]->Location, 1, &Value[0]);
                break;
            }
            case EType::MAT4:
            {
                glm::mat4 Value;
                glGetUniformfv(OldProgramId, OldUniformLocation, &Value[0][0]);
                glUniformMatrix4fv(GLRecompiledShader->uniformVariables[i]->Location, 1, GL_FALSE, &Value[0][0]);
                break;
            }
            }
        }

        // Replace uniforms
        uniformVariables.Free();
        uniformVariables = GLRecompiledShader->uniformVariables;

        // Preserve texture bindings
        // @TODO n^2, might not be a problem, as we're not goin to ship it
        for (u32 i = 0; i < GLRecompiledShader->textureBindings.GetLength(); ++i)
        {
            for (u32 j = 0; j < textureBindings.GetLength(); ++j)
            {
                if (GLRecompiledShader->textureBindings[i]->Name == textureBindings[j]->Name)
                {
                    GLRecompiledShader->textureBindings[i]->BoundTexture = textureBindings[j]->BoundTexture;
                    GLRecompiledShader->textureBindings[i]->TextureIndex = textureBindings[j]->TextureIndex;
                }
            }
        }

        // Replace texture binding
        textureBindings.Free();
        textureBindings = GLRecompiledShader->textureBindings;

        RestoreTextureBindings();
        // Restore the currently bound shader's state
        if (CurrentShader)
        {
            CurrentShader->Use();
            CurrentShader->RestoreTextureBindings();
            CurrentShader->SetupBuffersBindings();
        }

        GLRecompiledShader->glShaderID = 0;
        glDeleteProgram(OldProgramId);
    }

    void CGLShader::SetupBuffersBindings()
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

    u32 CGLShader::GetIdForUniform(const FANSIString& InUniformName) const
    {
        for (u32 idx = 0; idx < uniformVariables.GetLength(); ++idx)
        {
            if (uniformVariables[idx]->Name == InUniformName)
                return idx;
        }

#ifndef NDEBUG
        if (warnMissingUniforms)
        {
            // LUCID_LOG(LogLevel::WARN, "Uniform variable with name %s not found in shader %s\n", *Name, *this->ShaderName);
        }
#endif
        return uniformVariables.GetLength();
    };

    u32 CGLShader::GetTextureId(const FANSIString& InUniformName) const
    {
        assert(gpu::Info.CurrentShader == this);

        for (u32 idx = 0; idx < textureBindings.GetLength(); ++idx)
        {
            if (textureBindings[idx]->Name == InUniformName)
            {
                return idx;
            }
        }

#ifndef NDEBUG
        if (warnMissingUniforms)
        {
            // LUCID_LOG(LogLevel::WARN, "Sampler with name %s not found in shader %s\n", *Name, *this->ShaderName);
        }
#endif
        return textureBindings.GetLength();
    }

    void CGLShader::Free()
    {
        if (glShaderID)
        {
            glDeleteProgram(glShaderID);
        }
    }
} // namespace lucid::gpu