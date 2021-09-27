﻿STRUCT_BEGIN(lucid, FActorEntry, "Description of the actor in the world")
    STRUCT_FIELD(u32, Id, 0, "Id of the actor")	
    STRUCT_FIELD(u32, ParentId, 0, "Id of the parent actor")	
    STRUCT_FIELD(FDString, Name, "", "Name of the actor")	
    STRUCT_FIELD(UUID, BaseActorResourceId, sole::INVALID_UUID, "Id of the base actor resource that this actor is an instance of")	
    STRUCT_STATIC_ARRAY(float, Postion, 3, {0 COMMA 0 COMMA 0 }, "Position of the actor")	
    STRUCT_STATIC_ARRAY(float, Scale, 3, {1 COMMA 1 COMMA 1 }, "Scale of the actor")	
    STRUCT_STATIC_ARRAY(float, Rotation, 4, { 0 COMMA 0 COMMA 0 COMMA 1 }, "Rotation (quat x, y, z, w) of the actor")
    STRUCT_FIELD(bool, bVisible, true, "")	    
STRUCT_END()

STRUCT_INHERIT_BEGIN(lucid, FStaticMeshDescription, lucid::FActorEntry, "")
    STRUCT_FIELD(InstancedVariable<UUID>, MeshResourceId, lucid::InstancedVariable<lucid::UUID>{}, "")	
    STRUCT_DYNAMIC_ARRAY(InstancedVariable<UUID>, MaterialIds, "")
    STRUCT_FIELD(bool, bReverseNormals, false, "")
    STRUCT_FIELD(scene::EStaticMeshType, Type, lucid::scene::EStaticMeshType::STATIONARY, 0, "")
STRUCT_END()

STRUCT_INHERIT_BEGIN(lucid, FTerrainDescription, lucid::FActorEntry, "")
    STRUCT_FIELD(UUID, TerrainMeshResourceId, lucid::UUID{}, "")
    STRUCT_FIELD(UUID, TerrainMaterialId, lucid::UUID{}, "")
    STRUCT_FIELD(float, ResolutionX, 0, "")
    STRUCT_FIELD(float, ResolutionZ, 0, "")    
    STRUCT_FIELD(float, GridSizeX, 0, "")    
    STRUCT_FIELD(float, GridSizeZ, 0, "")
    STRUCT_FIELD(bool,  bFlat, false, "")
    STRUCT_FIELD(i32,   Seed, 6, "")
    STRUCT_FIELD(i32,   Octaves, 6, "")
    STRUCT_FIELD(float, Frequency, 0.005f, "")
    STRUCT_FIELD(float, Amplitude, 1.0f, "")
    STRUCT_FIELD(float, Lacunarity, 4.152f, "")
    STRUCT_FIELD(float, Persistence, 0.122f, "")
    STRUCT_FIELD(float, MinHeight, 0.f, "")
    STRUCT_FIELD(float, MaxHeight, 5.f, "")
STRUCT_END()

STRUCT_INHERIT_BEGIN(lucid, FSkyboxDescription, lucid::FActorEntry, "")
    STRUCT_FIELD(InstancedVariable<UUID>, RightFaceTextureID, lucid::InstancedVariable<lucid::UUID>{}, "")	
    STRUCT_FIELD(InstancedVariable<UUID>, LeftFaceTextureID, lucid::InstancedVariable<lucid::UUID>{}, "")	
    STRUCT_FIELD(InstancedVariable<UUID>, TopFaceTextureID, lucid::InstancedVariable<lucid::UUID>{}, "")	
    STRUCT_FIELD(InstancedVariable<UUID>, BottomFaceTextureID, lucid::InstancedVariable<lucid::UUID>{}, "")	
    STRUCT_FIELD(InstancedVariable<UUID>, FrontFaceTextureID, lucid::InstancedVariable<lucid::UUID>{}, "")	
    STRUCT_FIELD(InstancedVariable<UUID>, BackFaceTextureID, lucid::InstancedVariable<lucid::UUID>{}, "")	
    STRUCT_FIELD(int, Width, 0, "")	
    STRUCT_FIELD(int, Height, 0, "")	
STRUCT_END()

STRUCT_BEGIN(lucid, FLightEntry, "")
    STRUCT_FIELD(u32, Id, 0, "Id of the light")	
    STRUCT_FIELD(u32, ParentId, 0, "Parent actor id")	
    STRUCT_STATIC_ARRAY(float, Color, 3, {1 COMMA 1 COMMA 1 }, "")	
    STRUCT_FIELD(u8, Quality, 1, "")	
    STRUCT_FIELD(FDString, Name, "", "Name of the actor")	
    STRUCT_STATIC_ARRAY(float, Postion, 3, {0 COMMA 0 COMMA 0 }, "Position of the actor")	
    STRUCT_STATIC_ARRAY(float, Rotation, 4, { 0 COMMA 0 COMMA 0 COMMA 0 }, "Rotation (quat) of the actor")
    STRUCT_FIELD(bool, bCastsShadow, false, "")	    
STRUCT_END()

STRUCT_INHERIT_BEGIN(lucid, FDirectionalLightEntry, lucid::FLightEntry, "")
    STRUCT_STATIC_ARRAY(float, Direction, 3, {0 COMMA 0 COMMA 1}, "")	
    STRUCT_STATIC_ARRAY(float, LightUp, 3, {0 COMMA 1 COMMA 0}, "")
    STRUCT_FIELD(float, Illuminance, 10, "")
    STRUCT_FIELD(int,   CascadeCount, 3, "")
    STRUCT_FIELD(float, CascadeSplitLogFactor, 0.9, "")
    STRUCT_FIELD(float, FirstCascadeNearPlane, 0.1, "")
    STRUCT_FIELD(float, ShadowsMaxDistance, 1000, "")
    STRUCT_FIELD(float, ShadowsZMuliplier, 10, "")
STRUCT_END()

STRUCT_INHERIT_BEGIN(lucid, FSpotLightEntry, lucid::FLightEntry, "")
    STRUCT_STATIC_ARRAY(float, Direction, 3, {0 COMMA 0 COMMA 1}, "")	
    STRUCT_STATIC_ARRAY(float, LightUp, 3, {0 COMMA 1 COMMA 0}, "")	
    STRUCT_FIELD(float, AttenuationRadius, 50, "")	
    STRUCT_FIELD(float, InnerCutOffRad, 0, "")	
    STRUCT_FIELD(float, OuterCutOffRad, 0, "")
    STRUCT_FIELD(lucid::scene::ELightSourceType, LightSourceType, lucid::scene::ELightSourceType::INCANDESCENT, "")
    STRUCT_FIELD(lucid::scene::ELightUnit, LightUnit, lucid::scene::ELightUnit::LUMENS, "")
    STRUCT_FIELD(float, LuminousPower, 620.f, "")
    STRUCT_FIELD(float, RadiantPowery, 8.f, "")
STRUCT_END()

STRUCT_INHERIT_BEGIN(lucid, FPointLightEntry, lucid::FLightEntry, "")
    STRUCT_FIELD(float, AttenuationRadius, 0, "")
    STRUCT_FIELD(lucid::scene::ELightSourceType, LightSourceType, lucid::scene::ELightSourceType::INCANDESCENT, "")
    STRUCT_FIELD(lucid::scene::ELightUnit, LightUnit, lucid::scene::ELightUnit::LUMENS, "")
    STRUCT_FIELD(float, LuminousPower, 620.f, "")
    STRUCT_FIELD(float, RadiantPowery, 8.f, "")
STRUCT_END()

STRUCT_BEGIN(lucid, FWorldDescription, "Listing of all the things in the world")
    STRUCT_DYNAMIC_ARRAY(lucid::FStaticMeshDescription, StaticMeshes, "")
    STRUCT_FIELD(lucid::FSkyboxDescription, Skybox, lucid::FSkyboxDescription{}, "Paths to the skybox actor asset")
    STRUCT_DYNAMIC_ARRAY(lucid::FDirectionalLightEntry, DirectionalLights, "")
    STRUCT_DYNAMIC_ARRAY(lucid::FSpotLightEntry, SpotLights, "")
    STRUCT_DYNAMIC_ARRAY(lucid::FPointLightEntry, PointLights, "")
    STRUCT_DYNAMIC_ARRAY(lucid::FTerrainDescription, Terrains, "")
STRUCT_END()

STRUCT_BEGIN(lucid, FActorDatabaseEntry, "")
    STRUCT_FIELD(UUID,     ActorId, sole::INVALID_UUID, "")
    STRUCT_FIELD(FDString, ActorPath, "", "")
    STRUCT_FIELD(lucid::scene::EActorType, ActorType, lucid::scene::EActorType::UNKNOWN, "")
STRUCT_END()

STRUCT_BEGIN(lucid, FActorDatabase, "")
    STRUCT_DYNAMIC_ARRAY(FActorDatabaseEntry, Entries,  "")
STRUCT_END()
