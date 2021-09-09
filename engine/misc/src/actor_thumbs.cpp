#include "misc/actor_thumbs.hpp"

#include <devices/gpu/viewport.hpp>


#include "engine/engine.hpp"
#include "devices/gpu/shader.hpp"
#include "devices/gpu/texture_enums.hpp"
#include "devices/gpu/vao.hpp"
#include "resources/mesh_resource.hpp"
#include "scene/actors/static_mesh.hpp"

#include "common/strings.hpp"
#include "devices/gpu/framebuffer.hpp"
#include "devices/gpu/texture.hpp"

namespace lucid
{
    static const FSString MODEL_MATRIX("uModel");
    static const FSString VIEW_MATRIX("uView");
    static const FSString PROJECTION_MATRIX("uProjection");

    void CActorThumbsGenerator::Setup()
    {
        Framebuffer = gpu::CreateFramebuffer(FSString{ "ActorThumbFramebuffer" });
        Framebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);

        Renderbuffer = gpu::CreateRenderbuffer(gpu::ERenderbufferFormat::DEPTH24_STENCIL8, { 1280, 720 }, FSString{ "ActorThumbFramebuffer" });
        Renderbuffer->Bind();
        Framebuffer->SetupDepthStencilAttachment(Renderbuffer);

        MeshThumbShader = GEngine.GetShadersManager().GetShaderByName("MeshThumb");

        PipelineState.ClearColorBufferColor = FColor{ 0.15 };
        PipelineState.ClearDepthBufferValue = 0;
        PipelineState.IsDepthTestEnabled = true;
        PipelineState.DepthTestFunction = gpu::EDepthTestFunction::LEQUAL;
        PipelineState.IsBlendingEnabled = false;
        PipelineState.IsCullingEnabled = false;
        PipelineState.IsSRGBFramebufferEnabled = false;
        PipelineState.IsDepthBufferReadOnly = false;

        Camera.Yaw = -90.f;
        Camera.UpdateCameraVectors();

    }

    // @TODO Fix
    gpu::CTexture* CActorThumbsGenerator::GenerateMeshThumb(const u16& InWidth, const u16& InHeight, resources::CMeshResource* MeshResource)
    {
        // Create the texture that will hold the result
        FDString        ThumbTextureName = SPrintf("%s_Thumb", *MeshResource->GetName());
        gpu::CTexture*  ThumbTexture = gpu::CreateEmpty2DTexture(InWidth, InHeight, gpu::ETextureDataType::UNSIGNED_BYTE, gpu::ETextureDataFormat::RGB, gpu::ETexturePixelFormat::RGB, 0, ThumbTextureName);

        // Prepare the framebuffer
        Framebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);

        ThumbTexture->Bind();
        ThumbTexture->SetMinFilter(gpu::EMinTextureFilter::NEAREST);
        ThumbTexture->SetMagFilter(gpu::EMagTextureFilter::NEAREST);

        Framebuffer->SetupColorAttachment(0, ThumbTexture);

        PipelineState.Viewport = { 0, 0, InWidth, InHeight };
        gpu::ConfigurePipelineState(PipelineState);
        gpu::ClearBuffers((gpu::EGPUBuffer)(gpu::EGPUBuffer::COLOR | gpu::EGPUBuffer::DEPTH));

        Camera.AspectRatio = (float)InWidth / (float)InHeight;

        MeshThumbShader->Use();

        // Calculate camera fov so the whole mesh fits in the view

        // const float VerticalMidPoint = (MeshResource->MaxPosY + MeshResource->MinPosY) / 2.0f;

        // Let's move the mesh a bit in view-space
        // const float MeshMaxZ = 10 + MeshResource->MaxPosZ;

        // const float FovX = fmaxf(atan2f(fabs(MeshResource->MinPosX), MeshMaxZ), atan2f(fabs(MeshResource->MaxPosX), MeshMaxZ)); 
        // const float FovY = fmaxf(atan2f(fabs(MeshResource->MinPosY), MeshMaxZ), atan2f(fabs(MeshResource->MaxPosY), MeshMaxZ)); 

        // Camera.Position = { 0, VerticalMidPoint, 10 };
        // Camera.FOV = glm::degrees(fmax(FovX, FovY));
        
        MeshThumbShader->SetMatrix(PROJECTION_MATRIX, Camera.GetProjectionMatrix());
        MeshThumbShader->SetMatrix(VIEW_MATRIX, Camera.GetViewMatrix());

        for (u16 i = 0; i < MeshResource->SubMeshes.GetLength(); ++i)
        {
            MeshResource->SubMeshes[i]->VAO->Bind();
            MeshResource->SubMeshes[i]->VAO->Draw();
        }

        return ThumbTexture; 
    }
}
