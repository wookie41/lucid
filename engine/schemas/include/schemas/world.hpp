STRUCT_BEGIN(lucid, FActorDescription, "")
    STRUCT_FIELD(FDString, Name, "", "Name of the actor")	
    STRUCT_STATIC_ARRAY(float, Postion, 3, {0 COMMA 0 COMMA 0 }, "Position of the actor")	
    STRUCT_STATIC_ARRAY(float, Scale, 3, {1 COMMA 1 COMMA 1 }, "Scale of the actor")	
    STRUCT_STATIC_ARRAY(float, Rotation, 4, { 0 COMMA 0 COMMA 0 COMMA 0 }, "Rotation (quat) of the actor")	
STRUCT_END()

STRUCT_INHERIT_BEGIN(lucid, FStaticMeshDescription, FActorDescription, "")
    STRUCT_FIELD(FDString, MeshResourceName, "", "")	
    STRUCT_FIELD(FDString, MaterialName, "", "")	
STRUCT_END()

STRUCT_INHERIT_BEGIN(lucid, FStaticMeshDescription, FActorDescription, "")
    STRUCT_FIELD(FDString, MeshResourceName, "", "")	
    STRUCT_FIELD(FDString, MaterialName, "", "")	
STRUCT_END()


STRUCT_BEGIN(lucid, FWorldDescription, "Listing of all the things in the world")
STRUCT_END()

