#pragma once

namespace lucid
{
    /**
     *  Interface class used when some we need hooks into the engine lifecycle.
     *  We can add an object to engine's EngineObjects array end our functions will be called by the engine
     */
    class IEngineObject
    {
    public:
        virtual void OnFrameBegin() {};
        virtual void OnFrameEnd() {};
    };
}
