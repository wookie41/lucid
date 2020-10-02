#include "canvas/canvas.hpp"

#include "devices/gpu/shader.hpp"
#include "common/strings.hpp"
#include "graphics/static_mesh.hpp"
#include "devices/gpu/texture.hpp"

namespace lucid::canvas
{

    static const lucid::String ITEM_POSITION{ "Position" };
    static const lucid::String ITEM_ROTATION{ "Rotation" };

    static const lucid::String SPRITE_TEXTURE{ "SpriteTexture" };
    static const lucid::String SPRITE_SIZE{ "SpriteSize" };
    static const lucid::String TEXTURE_SIZE{ "TextureSize" };
    static const lucid::String TEXTURE_REGION_SIZE{ "TextureRegionSize" };
    static const lucid::String TEXTURE_REGION_INDEX{ "TextureRegionIndex" };

    void CanvasItem::PrepareShader(gpu::Shader* ShaderToUse)
    {
        float rotation = Rotation;
        glm::vec2 position = Position;

        if (Parent)
        {
            if (RespectParentRotation)
                rotation += Parent->Rotation;

            if (RespectParentPosition)
                position += Parent->Position;
        }

        //@TODO use to rotation
        // ShaderToUse->SetFloat(ITEM_ROTATION, rotation);
        ShaderToUse->SetVector(ITEM_POSITION, position);
    }

    void CanvasItem::Draw(gpu::Shader* ShaderToUse)
    {
        if (!IsVisible)
            return;

        LinkedListItem<CanvasItem>* current = &Children.Head;
        while (current && current->Element)
        {
            current->Element->Draw(ShaderToUse);
            current = current->Next;
        }
    }

    void CanvasItem::AddChild(CanvasItem* Item)
    {
        Children.Add(Item);
        Item->Parent = this;
    }
    void CanvasItem::RemoveChild(CanvasItem* Item) { Children.Remove(Item); }

    void Sprite::Draw(gpu::Shader* ShaderToUse)
    {
        CanvasItem::PrepareShader(ShaderToUse);

        if (TextureToUse)
        {
            const auto dimensions = TextureToUse->GetDimensions();
            ShaderToUse->SetVector(TEXTURE_SIZE, glm::ivec2{ dimensions.x, dimensions.y });
            ShaderToUse->UseTexture(SPRITE_TEXTURE, TextureToUse);
        }
        ShaderToUse->SetVector(SPRITE_SIZE, SpriteSize);
        ShaderToUse->SetVector(TEXTURE_REGION_SIZE, TextureRegionSize);
        ShaderToUse->SetVector(TEXTURE_REGION_INDEX, TextureRegionIndex);

        lucid::graphics::DrawMesh(&lucid::graphics::QuadShape);

        CanvasItem::Draw(ShaderToUse);

        if (TextureToUse)
        {
            TextureToUse->Unbind();
        }
    }
} // namespace lucid::canvas
