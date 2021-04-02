STRUCT_BEGIN(lucid::schema, FScene, "Describies the scene itself and resources needed to render it")
    STRUCT_DYNAMIC_ARRAY(FTextureResource, Textures, "Textures neede by the scene")
STRUCT_END()