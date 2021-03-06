#include "scene/blinn_phong_material.hpp"
#include "devices/gpu/shader.hpp"
#include "common/log.hpp"
#include "devices/gpu/texture.hpp"
#include "resources/mesh.hpp"
#include "resources/texture.hpp"
#include "scene/renderable.hpp"

namespace lucid::scene
{
    static const String SHININESS("uMaterial.Shininess");
    static const String DIFFUSE_COLOR("uMaterial.DiffuseColor");
    static const String SPECULAR_COLOR("uMaterial.SpecularColor");

    BlinnPhongMaterial::BlinnPhongMaterial(gpu::Shader* CustomShader) : Material(CustomShader) {}

    void BlinnPhongMaterial::SetupShader(gpu::Shader* Shader)
    {
        Shader->SetInt(SHININESS, Shininess);
        Shader->SetVector(DIFFUSE_COLOR, DiffuseColor);
        Shader->SetVector(DIFFUSE_COLOR, SpecularColor);
    };

    /* ---------------------------------------------------------------------------*/

    static const String DIFFUSE_MAP("uMaterial.DiffuseMap");
    static const String SPECULAR_MAP("uMaterial.SpecularMap");
    static const String NORMAL_MAP("uMaterial.NormalMap");
    static const String HAS_SPECULAR_MAP("uMaterial.HasSpecularMap");
    static const String HAS_NORMAL_MAP("uMaterial.HasNormalMap");
    static const String HAS_DISPLACEMENT_MAP("uMaterial.HasDisplacementMap");
    static const String DISPLACEMENT_MAP("uMaterial.DisplacementMap");

    BlinnPhongMapsMaterial::BlinnPhongMapsMaterial(gpu::Shader* CustomShader) : Material(CustomShader) {}

    void BlinnPhongMapsMaterial::SetupShader(gpu::Shader* Shader)
    {
        Shader->SetInt(SHININESS, Shininess);
        Shader->UseTexture(DIFFUSE_MAP, DiffuseMap);

        if (SpecularMap != nullptr)
        {
            Shader->UseTexture(SPECULAR_MAP, SpecularMap);
            Shader->SetBool(HAS_SPECULAR_MAP, true);
        }
        else
        {
            Shader->SetBool(HAS_SPECULAR_MAP, false);
            Shader->SetVector(SPECULAR_COLOR, SpecularColor);
        }

        if (NormalMap != nullptr)
        {
            Shader->UseTexture(NORMAL_MAP, NormalMap);
            Shader->SetBool(HAS_NORMAL_MAP, true);
        }
        else
        {
            Shader->SetBool(HAS_NORMAL_MAP, false);
        }
        
        if (DisplacementMap != nullptr)
        {
            Shader->UseTexture(DISPLACEMENT_MAP, DisplacementMap);
            Shader->SetBool(HAS_DISPLACEMENT_MAP, true);
        }
        else
        {
            Shader->SetBool(HAS_DISPLACEMENT_MAP, false);
        }
    };


    Renderable* CreateBlinnPhongRenderable(DString InMeshName, resources::MeshResource* InMesh, gpu::Shader* InShader)
        {
            gpu::Texture* FallbackTexture = resources::TexturesHolder.GetDefaultResource()->TextureHandle;

            BlinnPhongMapsMaterial* MeshMaterial = new BlinnPhongMapsMaterial(InShader);
            MeshMaterial->Shininess = 32;
    
            if (InMesh->DiffuseMap == nullptr)
            {
                LUCID_LOG(LogLevel::INFO, "Mesh is missing a diffuse map");
                MeshMaterial->DiffuseMap = FallbackTexture;
            }
            else
            {
                MeshMaterial->DiffuseMap = InMesh->DiffuseMap->TextureHandle;
            }
    
            if (InMesh->SpecularMap == nullptr)
            {
                LUCID_LOG(LogLevel::INFO, "Mesh is missing a specular map");
                MeshMaterial->SpecularMap = FallbackTexture;
            }
            else
            {
                MeshMaterial->SpecularMap = InMesh->SpecularMap->TextureHandle;
            }
    
            if (InMesh->NormalMap == nullptr)
            {
                LUCID_LOG(LogLevel::INFO, "Mesh is missing a normal map");
                MeshMaterial->NormalMap = FallbackTexture;
            }
            else
            {
                MeshMaterial->NormalMap = InMesh->NormalMap->TextureHandle;
            }
    
            Renderable* MeshRenderable = new Renderable{ InMeshName };
            MeshRenderable->Material = MeshMaterial;
            MeshRenderable->Type = RenderableType::STATIC;
            MeshRenderable->VertexArray = InMesh->VAO;
    
            return MeshRenderable;
        }
} // namespace lucid::scene