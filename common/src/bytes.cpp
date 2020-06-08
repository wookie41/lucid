#include "common/bytes.hpp"

namespace lucid
{
    void zero(void* Address, const uint64_t& NumBytes)
    {
        //TODO this can be done more efficiently with intrinsicts that will omit the CPU cache
        char* p = (char*)Address;
        for (uint32_t i = 0; i < NumBytes; ++i) 
        {
            p[i] = 0;
        }
    }
    
} // namespace lucid
