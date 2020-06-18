#pragma once

#include "common/collections.hpp"
#include "common/math.hpp"

namespace lucid::gpu
{
    class Shader;
    class Texture;
} // namespace lucid::gpu

namespace lucid::canvas
{
    class Drawable
    {
      public:
        virtual void Draw(gpu::Shader* ShaderToUse) = 0;
    };

    class CanvasItem : public Drawable
    {
      public:
        virtual void Draw(gpu::Shader* ShaderToUse) override;

        void AddChild(CanvasItem* Item);
        void RemoveChild(CanvasItem* Item);

        bool RespectParentRotation = true;
        bool RespectParentPosition = true;

        math::vec2 Position;
        float Rotation = 0;

        CanvasItem* Parent = nullptr;
        LinkedList<CanvasItem> Children;

      protected:
        virtual void PrepareShader(gpu::Shader* ShaderToUse);
    };

    class Sprite : public CanvasItem
    {
      public:
        virtual void Draw(gpu::Shader* ShaderToUse) override;

        math::ivec2 SpriteSize;
        math::ivec2 TextureRegionSize;
        math::ivec2 TextureRegionIndex;

        gpu::Texture* TextureToUse = nullptr;
    };

} // namespace lucid::canvas