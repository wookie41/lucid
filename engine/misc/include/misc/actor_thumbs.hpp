#pragma once


#include "devices/gpu/gpu.hpp"
#include "scene/camera.hpp"

namespace lucid
{
    namespace scene
    {
        class CRenderer;
        class CStaticMesh;
    };

    namespace gpu
    {
        class CTexture;
        class CFramebuffer;
        class CRenderbuffer;
        class CShader;
        class CVertexArray;
    };

    namespace resources
    {
        class CMeshResource;
    }
    
    class CActorThumbsGenerator
    {
    public:

        void Setup();

        gpu::CTexture* GenerateMeshThumb(const u16& InWidth, const u16& InHeight, resources::CMeshResource* MeshResource);

    private:

        scene::CCamera      Camera { scene::ECameraMode::PERSPECTIVE };

        gpu::CShader*       MeshThumbShader = nullptr;

        gpu::FPipelineState PipelineState { };
        gpu::CFramebuffer*  Framebuffer = nullptr;
        gpu::CRenderbuffer* Renderbuffer = nullptr;
    };    
} // namespace lucid
