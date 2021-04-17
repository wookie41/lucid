#include "devices/gpu/gpu_object.hpp"

namespace lucid::gpu
{
    CGPUObject::~CGPUObject()
    {
        Name.Free();
    }    
}
