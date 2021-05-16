#include "platform/sdl/sdl_window.hpp"

#include <devices/gpu/framebuffer.hpp>


#include "common/log.hpp"

#include "devices/gpu/gl/gl_framebuffer.hpp"
#include "devices/gpu/gpu.hpp"

#include "SDL2/SDL.h"

#include "imgui.h"
#include "ImGuizmo.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

namespace lucid::platform
{
    CWindow* CreateNewWindow(const FWindowDefiniton& Definition)
    {
        if (Definition.sRGBFramebuffer)
        {
            SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
        }
        Uint32 WindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE;
        if (Definition.bHidden)
        {
            WindowFlags |= SDL_WINDOW_HIDDEN;
        }
        SDL_Window* window = SDL_CreateWindow(Definition.title, Definition.X, Definition.Y, Definition.Width, Definition.Height, WindowFlags);
        if (window == nullptr)
        {
            LUCID_LOG(ELogLevel::ERR, "[SDL] Failed to create a new window: %s", SDL_GetError());
            return nullptr;
        }

        SDL_GLContext context = SDL_GL_CreateContext(window);
        if (context == nullptr)
        {
            SDL_DestroyWindow(window);
            LUCID_LOG(ELogLevel::ERR, "[SDL] Failed to create a context for the window: %s", SDL_GetError());
            return nullptr;
        }

        SDL_GL_MakeCurrent(window, context);
        SDL_CaptureMouse(SDL_FALSE);
        SDL_SetRelativeMouseMode(SDL_FALSE);
        // Set initial gpu state for this context
        gpu::DisableDepthTest();
        gpu::DisableBlending();
        gpu::DisableCullFace();
        
        auto* NewWindow = new SDLWindow(window, context, Definition.Width, Definition.Height);
        NewWindow->Init();
        return NewWindow;
    }

    SDLWindow::SDLWindow(SDL_Window* InWindow, SDL_GLContext InContext, const u16& InWidth, const u16& InHeight)
    : MySDLWindow(InWindow), MyGLContext(InContext), Width(InWidth), Height(InHeight), AspectRatio((float)InWidth / (float)InHeight)
    {
        WindowFramebuffer = new gpu::CGLDefaultFramebuffer{ InWidth, InHeight };
    }

    void SDLWindow::Init()
    {
        GPUStateForMyContext.BoundTextures = new gpu::CTexture*[gpu::GGPUInfo.MaxTextureUnits];
        for (int i = 0; i < gpu::GGPUInfo.MaxTextureUnits; ++i)
        {
            GPUStateForMyContext.BoundTextures[i] = nullptr;
        }
    }

    void SDLWindow::Swap() { SDL_GL_SwapWindow(MySDLWindow); }

    void SDLWindow::Clear()
    {
        WindowFramebuffer->Bind(gpu::EFramebufferBindMode::READ_WRITE);
        gpu::ClearBuffers((gpu::EGPUBuffer)(gpu::EGPUBuffer::COLOR | gpu::EGPUBuffer::DEPTH));
    }

    glm::vec2 SDLWindow::GetPosition() const
    {
        int WindowX, WindowY;
        SDL_GetWindowPosition(MySDLWindow, &WindowX, &WindowY);
        return { WindowX, WindowY };
    }

    void SDLWindow::Prepare()
    {
        gpu::GGPUState = &GPUStateForMyContext; 
        SDL_GL_MakeCurrent(MySDLWindow, MyGLContext);
    }

    u16 SDLWindow::GetWidth() const { return Width; }

    u16 SDLWindow::GetHeight() const { return Height; }

    float SDLWindow::GetAspectRatio() const { return AspectRatio; }

    gpu::CFramebuffer* SDLWindow::GetFramebuffer() const
    {
        return WindowFramebuffer;
    }

    void SDLWindow::Show() { SDL_ShowWindow(MySDLWindow); }

    void SDLWindow::Hide() { SDL_HideWindow(MySDLWindow); }

    void SDLWindow::Destroy()
    {
        delete WindowFramebuffer;
        SDL_GL_MakeCurrent(MySDLWindow, MyGLContext);
        SDL_GL_DeleteContext(MyGLContext);
        SDL_DestroyWindow(MySDLWindow);
    }

    void SDLWindow::OnFocusGained()
    {
        // SDL_CaptureMouse(SDL_TRUE);
    }

    void SDLWindow::OnResize(const u16& InWidth, const u16& InHeight)
    {
        Width = InWidth;
        Height = InHeight;
    }

    void SDLWindow::ImgUiSetup()
    {
        ImGui_ImplSDL2_InitForOpenGL(MySDLWindow, MyGLContext);
        ImGui_ImplOpenGL3_Init("#version 330 core");
    }

    void SDLWindow::ImgUiStartNewFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(MySDLWindow);
        ImGui::NewFrame();
    }

    void SDLWindow::ImgUiDrawFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
} // namespace lucid::platform