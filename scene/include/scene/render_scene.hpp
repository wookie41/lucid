#pragma once

#include "common/collections.hpp"

namespace lucid::scene
{
    // The RenderScene contains thing like objects to render, lights, fog voluems 
    // in a Renderer-implementation-agnostic format. The specific Renderers then
    // use the provided scene information to render the scene in their own specific way

    class Light;

    struct RenderScene
    {
        LinkedList<class Renderable> StaticGeometry;  
        LinkedList<class Renderable> DynamicGeometry;  
        LinkedList<Light> Lights; 
    };
}