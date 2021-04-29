STRUCT_BEGIN(lucid, FMaterialDescription, "")
    STRUCT_FIELD(FDString, Name, "", "Name of the material")	
    STRUCT_FIELD(FDString, ShaderName, "", "Name of the shader used by this material")	
STRUCT_END()

STRUCT_INHERIT_BEGIN(lucid, FFlatMaterialDescription, FMaterialDescription, "")
    STRUCT_STATIC_ARRAY(float, Color, 4, {0 COMMA 0 COMMA 0  COMMA 1}, "")	    
STRUCT_END()

STRUCT_INHERIT_BEGIN(lucid, FBlinnPhongMaterialDescription, FMaterialDescription, "")
    STRUCT_FIELD(int, Shininess, 32, "")	    
    STRUCT_STATIC_ARRAY(float, SpecularColor, 3, {0 COMMA 0 COMMA 0}, "")	    
    STRUCT_STATIC_ARRAY(float, DiffuseColor, 3, {0 COMMA 0 COMMA 0}, "")	    
STRUCT_END()

STRUCT_INHERIT_BEGIN(lucid, FBlinnPhongMapsMaterialDescription, FMaterialDescription, "")
    STRUCT_FIELD(int, Shininess, 32, "")	    
    STRUCT_FIELD(FDString, DiffuseTextureName, "", "")	    
    STRUCT_FIELD(FDString, SpecularTextureName, "", "")	    
    STRUCT_FIELD(FDString, NormalTextureName, "", "")
    STRUCT_FIELD(FDString, DisplacementTextureName, "", "")
    STRUCT_STATIC_ARRAY(float, SpecularColor, 3, {0 COMMA 0 COMMA 0}, "Fallback specular color when no specular map is present")	    
STRUCT_END()

